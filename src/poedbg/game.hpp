// Part of 'poedbg'. Copyright (c) 2018 maper. Copies must retain this attribution.

#pragma once

//////////////////////////////////////////////////////////////////////////
// Game Functions
//////////////////////////////////////////////////////////////////////////

/*
Uses the supplied details about a hook to search for the hook location in
the game's memory and calculate the hook properties.
*/
POEDBG_INLINE bool _PoeDbgGameSetHookProperties(PBYTE Signature, PULONG_PTR HookStart, PULONG_PTR HookEnd, ULONG_PTR HookOffset, ULONG_PTR HookSize)
{
	// Search for signature.
	*HookStart = _PoeDbgMemoryFind(Signature);

	if (NULL != *HookStart)
	{
		// Adjust start by offset.
		*HookStart += HookOffset;

		// Save off end address.
		*HookEnd = *HookStart + HookSize;
	}
	else
	{
		return false;
	}

	return true;
}

/*
Copies packet data from the game depending on the given buffer and size. Will
protect against buffer overflows.
*/
inline bool _PoeDbgGameCopyPacket(PBYTE LocalPacketBuffer, const DWORD64 PacketBuffer, const DWORD64 PacketLength)
{
	if (PacketLength > DEFAULT_BUFFER_SIZE)
	{
		return false;
	}

	if (!_PoeDbgMemoryRead(static_cast<ULONG_PTR>(PacketBuffer), LocalPacketBuffer, static_cast<SIZE_T>(PacketLength)))
	{
		return false;
	}

	return true;
}

/*
Actually sets the hooks for the given thread. This function assumes the
handle provided has permissions to modify the thread context.
*/
POEDBG_INLINE POEDBG_STATUS _PoeDbgGameSetHooksOnThread(const DWORD ThreadId, const HANDLE Thread)
{
	// Save off thread handle.
	_g_GameThreads[ThreadId] = Thread;

	if (!_PoeDbgMemorySetBreakpoint(Thread, _g_PacketSenderHookStart, BP_LENGTH_ONE, BP_CONDITION_EXECUTION, 0))
	{
		return POEDBG_STATUS_HOOK_SEND_FAILED;
	}

	if (!_PoeDbgMemorySetBreakpoint(Thread, _g_PacketRecvHookStart, BP_LENGTH_ONE, BP_CONDITION_EXECUTION, 1))
	{
		return POEDBG_STATUS_HOOK_RECV_FAILED;
	}

	if (!_PoeDbgMemorySetBreakpoint(Thread, _g_PacketWsaRecvHookStart, BP_LENGTH_ONE, BP_CONDITION_EXECUTION, 2))
	{
		return POEDBG_STATUS_HOOK_WSARECV_FAILED;
	}

	return POEDBG_STATUS_SUCCESS;
}

/*
Initializes any game hacking logic, i.e. performs pattern scans, makes any
required changes to the process to prepare for hooking. Actual hooks are
applied per-thread by the debug events, with the exception of the main
thread, which has its hooks applied in this handler.
*/
POEDBG_INLINE DWORD _PoeDbgGameInitializeProcess(const LPDEBUG_EVENT Event)
{
	// Save off the game handle.
	_g_GameHandle = Event->u.CreateProcessInfo.hProcess;

	// Save off the game base address.
	_g_GameBaseAddress = reinterpret_cast<ULONG_PTR>(Event->u.CreateProcessInfo.lpBaseOfImage);

	if (!_PoeDbgGameSetHookProperties(_g_PacketSenderPattern, &_g_PacketSenderHookStart, &_g_PacketSenderHookEnd, _g_PacketSenderHookOffset, _g_PacketSenderHookSize))
	{
		POEDBG_NOTIFY_CALLBACK(Error, POEDBG_STATUS_HOOK_PROPERTIES_SEND_FAILED);
	}

	if (!_PoeDbgGameSetHookProperties(_g_PacketRecvPattern, &_g_PacketRecvHookStart, &_g_PacketRecvHookEnd, _g_PacketRecvHookOffset, _g_PacketRecvHookSize))
	{
		POEDBG_NOTIFY_CALLBACK(Error, POEDBG_STATUS_HOOK_PROPERTIES_RECV_FAILED);
	}

	if (!_PoeDbgGameSetHookProperties(_g_PacketWsaRecvPattern, &_g_PacketWsaRecvHookStart, &_g_PacketWsaRecvHookEnd, _g_PacketWsaRecvHookOffset, _g_PacketWsaRecvHookSize))
	{
		POEDBG_NOTIFY_CALLBACK(Error, POEDBG_STATUS_HOOK_PROPERTIES_WSARECV_FAILED);
	}

	// Apply hooks on this initial main thread.
	POEDBG_STATUS Status = _PoeDbgGameSetHooksOnThread(Event->dwThreadId, Event->u.CreateProcessInfo.hThread);

	if (POEDBG_FAILURE(Status))
	{
		// Try to report error.
		POEDBG_NOTIFY_CALLBACK(Error, Status);
	}

	return DBG_CONTINUE;
}

/*
Initializes any per-thread hooking and other data. This is called by the
debug loop for every thread in the process, other than the main thread.
*/
POEDBG_INLINE DWORD _PoeDbgGameInitializeThread(const LPDEBUG_EVENT Event)
{
	// Apply hooks on this separate thread.
	POEDBG_STATUS Status = _PoeDbgGameSetHooksOnThread(Event->dwThreadId, Event->u.CreateThread.hThread);

	if (POEDBG_FAILURE(Status))
	{
		// Try to report error.
		POEDBG_NOTIFY_CALLBACK(Error, Status);
	}

	return DBG_CONTINUE;
}

/*
Processes a single step exception. Checks to see whether the exception belongs
to an existing hook, and reacts accordingly.
*/
POEDBG_INLINE DWORD _PoeDbgGameProcessHooks(const DWORD ThreadId, const EXCEPTION_DEBUG_INFO Exception)
{
	CONTEXT Context = { 0 };
	Context.ContextFlags = CONTEXT_ALL;

	// Get the saved handle for this thread.
	HANDLE Thread = _g_GameThreads[ThreadId];

	if (NULL == Thread)
	{
		// This is not a thread that we have set a hook on, so we won't handle
		// this exception.

		return DBG_EXCEPTION_NOT_HANDLED;
	}

	if (FALSE == GetThreadContext(Thread, &Context))
	{
		return DBG_EXCEPTION_NOT_HANDLED;
	}
	
	// Get the address where the exception occurred.
	ULONG_PTR ExceptionAddress = reinterpret_cast<ULONG_PTR>(Exception.ExceptionRecord.ExceptionAddress);

	if (ExceptionAddress == _g_PacketSenderHookStart)
	{
		// Is this exception coming from the packet sender hook? If so, 
		// we should record packet details and then re-execute anything
		// that we skipped with our hook.

		DWORD64 PacketBuffer = Context.Rdx;
		DWORD64 PacketLength = Context.R8;

		if (_PoeDbgGameCopyPacket(_g_PacketSenderBuffer, PacketBuffer, PacketLength))
		{
			// Forward the packet to the callback.
			POEDBG_NOTIFY_CALLBACK(PacketSend, static_cast<DWORD>(PacketLength), _g_PacketSenderBuffer[1], _g_PacketSenderBuffer);
		}

		// Execute skipped.
		Context.Rcx += 0x10;

		// Set the instruction pointer.
		Context.Rip = _g_PacketSenderHookEnd;
	}

	if (ExceptionAddress == _g_PacketRecvHookStart)
	{
		// Is this exception coming from the packet receiver hook? If so, 
		// we should record packet details and then re-execute anything
		// that we skipped with our hook.

		DWORD64 PacketBuffer = Context.R9;
		DWORD64 PacketLength = Context.Rax;

		if (_PoeDbgGameCopyPacket(_g_PacketRecvBuffer, PacketBuffer, PacketLength))
		{
			// Forward the packet to the callback.
			POEDBG_NOTIFY_CALLBACK(PacketReceive, static_cast<DWORD>(PacketLength), _g_PacketRecvBuffer[1], _g_PacketRecvBuffer);
		}

		// Execute skipped.
		Context.Rdi = Context.Rax;

		// Set the instruction pointer.
		Context.Rip = _g_PacketRecvHookEnd;
	}

	if (ExceptionAddress == _g_PacketWsaRecvHookStart)
	{
		// Is this exception coming from the WSA packet receiver hook? If so, 
		// we should record packet details and then re-execute anything
		// that we skipped with our hook.

		DWORD64 PacketBuffer = NULL;
		DWORD64 PacketLength = Context.Rdi;

		// Get the location of the buffer off the stack.
		_PoeDbgMemoryRead(static_cast<ULONG_PTR>(Context.Rsp + 0x48), &PacketBuffer, sizeof(DWORD64));

		if (_PoeDbgGameCopyPacket(_g_PacketWsaRecvBuffer, PacketBuffer, PacketLength))
		{
			// Forward the packet to the callback.
			POEDBG_NOTIFY_CALLBACK(PacketReceive, static_cast<DWORD>(PacketLength), _g_PacketWsaRecvBuffer[1], _g_PacketWsaRecvBuffer);
		}

		// Execute skipped.
		Context.Rax = Context.Rdi;

		// Set the instruction pointer.
		Context.Rip = _g_PacketWsaRecvHookEnd;
	}

	// Set the context.
	Context.ContextFlags = CONTEXT_ALL;

	if (FALSE == SetThreadContext(Thread, &Context))
	{
		return DBG_EXCEPTION_NOT_HANDLED;
	}

	return DBG_CONTINUE;
}