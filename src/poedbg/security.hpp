// Part of 'poedbg'. Copyright (c) 2018 maper. Copies must retain this attribution.

#pragma once

//////////////////////////////////////////////////////////////////////////
// Security Functions
//////////////////////////////////////////////////////////////////////////

/*
Tries to elevate the current process's privileges so that we are able to
attach to any process as a debugger.
*/
POEDBG_INLINE POEDBG_STATUS _PoeDbgSecurityChangePrivileges()
{
	HANDLE Token;
	LUID Luid;

	// Try to open our process token.
	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &Token);

	if (FALSE == LookupPrivilegeValueW(NULL, SE_DEBUG_NAME, &Luid))
	{
		// The privilege value SE_DEBUG_NAME may not be found on this
		// machine, which would be strange.
		return POEDBG_STATUS_PRIVILEGES_NOT_FOUND;
	}

	TOKEN_PRIVILEGES Tp;

	Tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	Tp.Privileges[0].Luid = Luid;
	Tp.PrivilegeCount = 1;

	// Try to adjust our privileges and then make sure that they
	// were assigned fully.

	if (FALSE == AdjustTokenPrivileges(Token, FALSE, &Tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
	{
		return POEDBG_STATUS_PRIVILEGES_NOT_ASSIGNED;
	}

	if (ERROR_NOT_ALL_ASSIGNED == GetLastError())
	{
		return POEDBG_STATUS_PRIVILEGES_NOT_ASSIGNED;
	}

	return POEDBG_STATUS_SUCCESS;
}

/*
Checks to see if the application was started as an administrator, as
we require that level of privilege.
*/
POEDBG_INLINE POEDBG_STATUS _PoeDbgSecurityGetPrivileges()
{
	// Initialize authority.
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

	// Initialize group.
	PSID AdministratorsGroup;

	// Initialize count.
	BYTE SubAuthorityCount = 2;

	// Allocate security identifier.
	BOOL bResult = AllocateAndInitializeSid(
		&NtAuthority,
		SubAuthorityCount,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&AdministratorsGroup);

	if (TRUE == bResult)
	{
		// Check if we belong to the administrators group.
		if (FALSE == CheckTokenMembership(NULL, AdministratorsGroup, &bResult))
		{
			bResult = FALSE;
		}

		// Free the security identifier.
		FreeSid(AdministratorsGroup);
	}

	return (bResult ? 
		POEDBG_STATUS_SUCCESS : 
		POEDBG_STATUS_PRIVILEGES_INSUFFICIENT);
}

/*
Iterates through all open processes until it finds one that contains the
target name, in which case it returns that process ID.
*/
POEDBG_INLINE DWORD _PoeDbgSecurityGetGameId(const wchar_t* Target)
{
	PROCESSENTRY32W Entry;

	// Take a snapshot of all processes in the system.
	HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (INVALID_HANDLE_VALUE == Snapshot)
	{
		return NULL;
	}

	// Set the size of the structure before using it.
	Entry.dwSize = sizeof(PROCESSENTRY32W);

	if (FALSE == Process32FirstW(Snapshot, &Entry))
	{
		// Close snapshot.
		CloseHandle(Snapshot);
		return NULL;
	}

	do
	{
		// Check if we found a process that has the same name as our target, and
		// if so we can return out of here.

		if (0 == lstrcmpW(Target, Entry.szExeFile))
		{
			// Close snapshot.
			CloseHandle(Snapshot);

			// Return the PID.
			return Entry.th32ProcessID;
		}
	} while (FALSE != Process32NextW(Snapshot, &Entry));

	// We didn't find it.
	CloseHandle(Snapshot);
	return NULL;
}
