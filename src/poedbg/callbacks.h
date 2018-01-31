// Part of 'poedbg'. Copyright (c) 2018 maper. Copies must retain this attribution.

#pragma once

//////////////////////////////////////////////////////////////////////////
// Macros
//////////////////////////////////////////////////////////////////////////

/*
A simple macro to create a global function pointer that represents a
callback with a particular name and type.
*/
#define POEDBG_CREATE_CALLBACK_POINTER(name, type) \
	__declspec(selectany) type _g_Callback##name;

/*
This macro creates both a registration and unregistration export for
the callback with the given name and type.
*/
#define POEDBG_CREATE_CALLBACK_EXPORTS(name, type) \
	POEDBG_EXPORT PoeDbgRegister##name##Callback(PVOID Callback) \
	{ \
		if (NULL != _g_Callback##name##) \
		{ \
			return POEDBG_STATUS_CALLBACK_ALREADY_REGISTERED; \
		} \
		_g_Callback##name = reinterpret_cast<##type##>(Callback); \
		return POEDBG_STATUS_SUCCESS; \
	} \
	POEDBG_EXPORT PoeDbgUnregister##name##Callback() \
	{ \
		_g_Callback##name = NULL; \
		return POEDBG_STATUS_SUCCESS; \
	}

/*
Calls the given callback, forwarding the parameters. If no callback
has been registered, this will do nothing.
*/
#define POEDBG_NOTIFY_CALLBACK(name, ...) \
	if (NULL != _g_Callback##name##) \
	{ \
		_g_Callback##name##(__VA_ARGS__); \
	}

//////////////////////////////////////////////////////////////////////////
// Callback Function Types
//////////////////////////////////////////////////////////////////////////

typedef void(__stdcall *POEDBG_ERROR_CALLBACK)(int Status);
typedef void(__stdcall *POEDBG_PACKET_CALLBACK)(unsigned int Length, BYTE Id, PBYTE Data);

//////////////////////////////////////////////////////////////////////////
// Callback Function Pointers
//////////////////////////////////////////////////////////////////////////

POEDBG_CREATE_CALLBACK_POINTER(Error, POEDBG_ERROR_CALLBACK)
POEDBG_CREATE_CALLBACK_POINTER(PacketSend, POEDBG_PACKET_CALLBACK)
POEDBG_CREATE_CALLBACK_POINTER(PacketReceive, POEDBG_PACKET_CALLBACK)