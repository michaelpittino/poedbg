// Part of 'poedbg'. Copyright (c) 2018 maper. Copies must retain this attribution.

#pragma once

// Exclude certain extra things from the Windows headers to speed
// up compile time.

#define WIN32_LEAN_AND_MEAN

// Configure whether we use inline suggestion or force functions
// to be inlined. Useful for debugging.

#define POEDBG_INLINE __forceinline

// Dramatically shorten the required function decorations for
// all of our exported functions.

#define POEDBG_EXPORT extern "C" __declspec(dllexport) POEDBG_STATUS __stdcall

// Here we'll include all of the standard includes we use in
// the module, ignoring their many warnings.

#pragma warning(push, 0)
#pragma warning(disable:4571)
#pragma warning(disable:4625)
#pragma warning(disable:4626)
#pragma warning(disable:4710)
#pragma warning(disable:4820)
#pragma warning(disable:5027)
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <map>
#pragma warning(pop) 
