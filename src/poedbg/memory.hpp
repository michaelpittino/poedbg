// Part of 'poedbg'. Copyright (c) 2018 maper. Copies must retain this attribution.

#pragma once

//////////////////////////////////////////////////////////////////////////
// Memory Functions
//////////////////////////////////////////////////////////////////////////

/*
Reads from the given game address into the buffer. Will return if the 
buffer is not sufficiently large.
*/
POEDBG_INLINE bool _PoeDbgMemoryRead(ULONG_PTR Address, PVOID Buffer, SIZE_T Size)
{
	if (NULL == _g_GameHandle)
	{
		return false;
	}

	// Try to read from the game.
	return (TRUE == ReadProcessMemory(_g_GameHandle, reinterpret_cast<LPCVOID>(Address), Buffer, Size, NULL));
}

/*
Writes to the game based on the provided buffer and size. Will return 
if the buffer is not sufficiently large.
*/
POEDBG_INLINE bool _PoeDbgMemoryWrite(ULONG_PTR Address, PVOID Buffer, SIZE_T Size)
{
	if (NULL == _g_GameHandle)
	{
		return false;
	}

	SIZE_T BytesWritten = 0;

	// Try to write to the game.
	return (TRUE == WriteProcessMemory(_g_GameHandle, reinterpret_cast<PVOID>(Address), Buffer, Size, &BytesWritten));
}

/*
Translates an address from the live game into an address in the same
location in our local copy of the game code.
*/
POEDBG_INLINE ULONG_PTR _PoeDbgMemoryGameAddressToLocal(ULONG_PTR GameAddress)
{
	// Calculate the base address of the code in the real game.
	ULONG_PTR GameCodeBase = _g_GameBaseAddress + _g_GameBaseOfCode;

	// Calculate how far into the game code the address is.
	ULONG_PTR GameDelta = GameAddress - GameCodeBase;

	// Return the address of the same delta into the local code copy.
	return (GameDelta + _g_GameCodeCopy);
}

/*
Translates an address from our local copy of the game code into an
equivalent offset within the live game code.
*/
POEDBG_INLINE ULONG_PTR _PoeDbgMemoryLocalAddressToGame(ULONG_PTR LocalAddress)
{
	// Calculate how far into the local code copy the address is.
	ULONG_PTR LocalDelta = LocalAddress - _g_GameCodeCopy;

	// Calculate the base address of the code in the real game.
	ULONG_PTR GameCodeBase = _g_GameBaseAddress + _g_GameBaseOfCode;

	// Return the address of the same delta into game code.
	return (LocalDelta + GameCodeBase);
}

/*
Retrieves a bunch of information about the target process such as code base,
dimensions, and other properties read from the PE header.
*/
POEDBG_INLINE POEDBG_STATUS _PoeDbgMemoryInitializeCache()
{
	if (!_PoeDbgMemoryRead(_g_GameBaseAddress, reinterpret_cast<PVOID*>(&_g_GameDosHeader), sizeof(IMAGE_DOS_HEADER)))
	{
		return POEDBG_STATUS_CACHE_DOS_HEADER_NOT_FOUND;
	}

	// Calculate the address of the NT headers within the game.
	ULONG_PTR NtHeadersAddress = _g_GameBaseAddress + _g_GameDosHeader.e_lfanew;

	if (!_PoeDbgMemoryRead(NtHeadersAddress, reinterpret_cast<PVOID*>(&_g_GameNtHeaders), sizeof(IMAGE_NT_HEADERS)))
	{
		return POEDBG_STATUS_CACHE_NT_HEADER_NOT_FOUND;
	}

	if (IMAGE_NT_SIGNATURE != _g_GameNtHeaders.Signature)
	{
		return POEDBG_STATUS_CACHE_NT_HEADER_INVALID;
	}

	// Save off the game image dimensions.
	_g_GameImageSize = _g_GameNtHeaders.OptionalHeader.SizeOfImage;
	_g_GameBaseOfCode = _g_GameNtHeaders.OptionalHeader.BaseOfCode;
	_g_GameSizeOfCode = _g_GameNtHeaders.OptionalHeader.SizeOfCode;

	// Allocate enough memory to store the game's .text section.
	_g_GameCodeCopy = reinterpret_cast<ULONG_PTR>(VirtualAlloc(NULL, _g_GameSizeOfCode, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));

	if (NULL == _g_GameCodeCopy)
	{
		return POEDBG_STATUS_CACHE_ALLOCATION_FAILED;
	}

	// Calculate the address of where the .text section starts.
	ULONG_PTR CodeBaseAddress = _g_GameBaseAddress + _g_GameBaseOfCode;

	if (!_PoeDbgMemoryRead(CodeBaseAddress, reinterpret_cast<PVOID*>(_g_GameCodeCopy), _g_GameSizeOfCode))
	{
		return POEDBG_STATUS_CACHE_COPY_FAILED;
	}

	return POEDBG_STATUS_SUCCESS;
}

/*
Search for the specified byte pattern starting from the given address and
ending with a null character. A required byte is prefixed by '_'. Also
supports logical AND-based comparisons if the byte is prefixed by '&'.
*/
POEDBG_INLINE ULONG_PTR _PoeDbgMemoryFindPattern(PBYTE Pattern, ULONG_PTR SearchAddress, SIZE_T SearchLength)
{
	// Re-cast search address.
	PBYTE SearchStart = reinterpret_cast<PBYTE>(SearchAddress);

	for (PBYTE This = SearchStart; This < &(SearchStart)[SearchLength]; This++)
	{
		for (PBYTE SearchIndex = This, PatternIndex = Pattern;; SearchIndex++, PatternIndex++)
		{
			if (NULL == PatternIndex[0])
			{
				// Specify where pattern was found.
				return reinterpret_cast<ULONG_PTR>(This);
			}

			if ('_' == PatternIndex[0])
			{
				if (SearchIndex[0] == PatternIndex[1])
				{
					PatternIndex++;
				}
				else
				{
					break;
				}
			}
			else if ('&' == PatternIndex[0])
			{
				if ((SearchIndex[0] & PatternIndex[1]) == PatternIndex[1])
				{
					PatternIndex++;
				}
				else
				{
					break;
				}
			}
		}
	}

	// Not found.
	return NULL;
}

/*
Finds the first instance of a given signature and returns it as a game address.
If the OverrideStartAddress parameter is used, starts search from that game address.
*/
POEDBG_INLINE ULONG_PTR _PoeDbgMemoryFind(PBYTE Pattern, ULONG_PTR OverrideSearchAddress = NULL)
{
	if (!_g_bIsGameInformationCaptured)
	{
		_g_bIsGameInformationCaptured = _PoeDbgMemoryInitializeCache();
	}

	// Initialize address.
	ULONG_PTR SearchAddress = NULL;

	if (OverrideSearchAddress != NULL)
	{
		// Start from the provided start address.
		SearchAddress = _PoeDbgMemoryGameAddressToLocal(OverrideSearchAddress);
	}
	else
	{
		// Start from the beginning.
		SearchAddress = _g_GameCodeCopy;
	}

	// Calculate the length of the search area.
	SIZE_T SearchLength = (_g_GameCodeCopy + _g_GameSizeOfCode) - SearchAddress;

	// Search for the signature.
	ULONG_PTR FoundAddress = _PoeDbgMemoryFindPattern(Pattern, SearchAddress, SearchLength);

	if (NULL != FoundAddress)
	{
		// Convert the located address to a game address.
		ULONG_PTR TranslatedAddress = _PoeDbgMemoryLocalAddressToGame(FoundAddress);

		// Return the address.
		return TranslatedAddress;
	}

	return NULL;
}

/*
Removes the hardware breakpoint set at the given index for the given thread
handle, if possible.
*/
POEDBG_INLINE bool _PoeDbgMemoryResetBreakpoint(HANDLE Thread, USHORT Index)
{
	if (Thread == NULL)
	{
		return false;
	}

	CONTEXT Context = { 0 };
	Context.ContextFlags = CONTEXT_DEBUG_REGISTERS;

	if (FALSE == GetThreadContext(Thread, &Context))
	{
		// First we need to get the original context so that if any debug registers
		// have been set, we make sure they stay set.

		return false;
	}

	// Reset local enabled bit for this register.
	_bittestandreset(reinterpret_cast<PLONG>(&Context.Dr7), static_cast<LONG>(Index * 2));

	if (FALSE == SetThreadContext(Thread, &Context))
	{
		return false;
	}

	return true;
}

/*
Sets the given debug register to monitor the given address, based on certain
conditions. This will overwrite any existing breakpoint.
*/
POEDBG_INLINE bool _PoeDbgMemorySetBreakpoint(HANDLE Thread, ULONG_PTR Address, SIZE_T Length, SIZE_T Type, USHORT Index)
{
	if (NULL == Thread)
	{
		return false;
	}

	CONTEXT Context = { 0 };
	Context.ContextFlags = CONTEXT_DEBUG_REGISTERS;

	if (FALSE == GetThreadContext(Thread, &Context))
	{
		// First we need to get the original context so that if any debug registers
		// have been set, we make sure they stay set.

		return false;
	}

	// Now we can set the correct debug register based on what
	// index the caller passed in.

	switch (Index)
	{
	case 0:
		Context.Dr0 = static_cast<DWORD64>(Address);
		break;
	case 1:
		Context.Dr1 = static_cast<DWORD64>(Address);
		break;
	case 2:
		Context.Dr2 = static_cast<DWORD64>(Address);
		break;
	case 3:
		Context.Dr3 = static_cast<DWORD64>(Address);
		break;
	default:
		return false;
	}

	// Set local enabled bit for this register.
	_bittestandset(reinterpret_cast<PLONG>(&Context.Dr7), static_cast<LONG>(Index * 2));

	// Set condition bits for this register.
	Context.Dr7 |= (Type << (static_cast<SIZE_T>(16) + (Index * 4)));

	// Set size bits for this register.
	Context.Dr7 |= (Length << (static_cast<SIZE_T>(18) + (Index * 4)));

	// Reset status register.
	Context.Dr6 = 0;

	if (FALSE == SetThreadContext(Thread, &Context))
	{
		return false;
	}

	return true;
}

/*
Sets the given hardware breakpoint on all threads within the process,
overwriting any existing hardware breakpoint at that index.
*/
__forceinline bool _PoeDbgMemoryModifyGlobalBreakpoint(ULONG_PTR Address, SIZE_T Length, SIZE_T Type, USHORT Index, bool bSet = true)
{
	// Take a snapshot of all running threads  
	HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	if (INVALID_HANDLE_VALUE == Snapshot)
	{
		return false;
	}

	// Initialize entry.
	THREADENTRY32 Entry;

	// Set the structure size.
	Entry.dwSize = sizeof(THREADENTRY32);

	if (FALSE == Thread32First(Snapshot, &Entry))
	{
		// Cleanup.
		CloseHandle(Snapshot);
		return false;
	}

	do
	{
		if (Entry.th32OwnerProcessID == _g_GameId)
		{
			// Try to open a handle to the thread.
			HANDLE Thread = OpenThread(THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME, FALSE, Entry.th32ThreadID);

			if (NULL == Thread)
			{
				continue;
			}

			// Suspend the thread.
			SuspendThread(Thread);

			if (bSet)
			{
				_PoeDbgMemorySetBreakpoint(Thread, Address, Length, Type, Index);
			}
			else
			{
				_PoeDbgMemoryResetBreakpoint(Thread, Index);
			}

			// Resume the thread.
			ResumeThread(Thread);
		}
	} while (FALSE != Thread32Next(Snapshot, &Entry));

	// Cleanup.
	CloseHandle(Snapshot);
	return true;
}