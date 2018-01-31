// Part of 'poedbg'. Copyright (c) 2018 maper. Copies must retain this attribution.

#include "common.h"
#include "globals.h"
#include "callbacks.h"
#include "security.hpp"
#include "memory.hpp"
#include "game.hpp"

//////////////////////////////////////////////////////////////////////////
// Exported API Functions
//////////////////////////////////////////////////////////////////////////

/*
Initializes the SDK for use. This must be called upon loading the host
module into a process that will use the API.
*/
POEDBG_EXPORT PoeDbgInitialize()
{
	POEDBG_RETURN_STATUS_ON_FAILURE(_PoeDbgSecurityGetPrivileges());
	POEDBG_RETURN_STATUS_ON_FAILURE(_PoeDbgSecurityChangePrivileges());

	// Try to get the PID of the game.
	_g_GameId = _PoeDbgSecurityGetGameId(GAME_PROCESS_NAME);

	if (NULL == _g_GameId)
	{
		// Try to get the PID of the game, steam version.
		_g_GameId = _PoeDbgSecurityGetGameId(GAME_PROCESS_NAME_STEAM);

		if (NULL == _g_GameId)
		{
			// If we're not able to get the PID, they probably haven't started the game
			// so we can't do much else here.

			return POEDBG_STATUS_GAME_NOT_FOUND;
		}

		// Using Steam.
		_g_bIsSteamClient = true;
	}

	// Start the debug loop.
	CreateThread(NULL, 0, DllDebugEventHandler, NULL, 0, 0);

	return POEDBG_STATUS_SUCCESS;
}

/*
Uninitializes the poedbg engine and detaches all hooks from the game, so that the
caller can safely close.
*/
POEDBG_EXPORT PoeDbgDestroy()
{
	if (NULL == _g_GameId)
	{
		return POEDBG_STATUS_GAME_NOT_FOUND;
	}

	// Remove hooks.
	_PoeDbgMemoryModifyGlobalBreakpoint(NULL, 0, 0, 0, false);
	_PoeDbgMemoryModifyGlobalBreakpoint(NULL, 0, 0, 1, false);
	_PoeDbgMemoryModifyGlobalBreakpoint(NULL, 0, 0, 2, false);

	// Stop the debugger.
	DebugActiveProcessStop(_g_GameId);

	if (NULL != _g_GameHandle)
	{
		// Release game handle.
		CloseHandle(_g_GameHandle);
	}

	if (NULL != _g_GameCodeCopy)
	{
		// Free our code copy.
		VirtualFree(reinterpret_cast<PVOID>(_g_GameCodeCopy), 0, MEM_FREE);

		// Reset pointers, etc.
		_g_GameBaseAddress = NULL;
		_g_GameCodeCopy = NULL;
		_g_GameImageSize = NULL;
		_g_GameBaseOfCode = NULL;
		_g_GameSizeOfCode = NULL;

		// Reset state.
		_g_bIsGameInformationCaptured = false;
		_g_bIsSteamClient = false;
	}

	return POEDBG_STATUS_SUCCESS;
}

// Here we list and construct all of the callback exports for registering
// and unregistering various callbacks.

POEDBG_CREATE_CALLBACK_EXPORTS(Error, POEDBG_ERROR_CALLBACK)
POEDBG_CREATE_CALLBACK_EXPORTS(PacketSend, POEDBG_PACKET_CALLBACK)
POEDBG_CREATE_CALLBACK_EXPORTS(PacketReceive, POEDBG_PACKET_CALLBACK)
