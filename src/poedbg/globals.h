// Part of 'poedbg'. Copyright (c) 2018 maper. Copies must retain this attribution.

#pragma once

//////////////////////////////////////////////////////////////////////////
// Type Definitions
//////////////////////////////////////////////////////////////////////////

// Status type.
typedef int POEDBG_STATUS;

//////////////////////////////////////////////////////////////////////////
// Status Codes
//////////////////////////////////////////////////////////////////////////

#define POEDBG_STATUS_HOOK_PROPERTIES_WSARECV_FAILED -20
#define POEDBG_STATUS_HOOK_PROPERTIES_RECV_FAILED -19
#define POEDBG_STATUS_HOOK_PROPERTIES_SEND_FAILED -18
#define POEDBG_STATUS_EXCEPTION_NOT_HANDLED -17
#define POEDBG_STATUS_CALLBACK_ALREADY_REGISTERED -16
#define POEDBG_STATUS_CALLBACK_NOT_SUPPORTED -15
#define POEDBG_STATUS_GAME_HOOK_BEHAVIOR_NOT_SET -14
#define POEDBG_STATUS_GAME_HOOK_NOT_SET -13
#define POEDBG_STATUS_GAME_NOT_FOUND -12
#define POEDBG_STATUS_HOOK_WSARECV_FAILED -11
#define POEDBG_STATUS_HOOK_RECV_FAILED -10
#define POEDBG_STATUS_HOOK_SEND_FAILED -9
#define POEDBG_STATUS_CACHE_COPY_FAILED -8
#define POEDBG_STATUS_CACHE_ALLOCATION_FAILED -7
#define POEDBG_STATUS_CACHE_NT_HEADER_INVALID -6
#define POEDBG_STATUS_CACHE_NT_HEADER_NOT_FOUND -5
#define POEDBG_STATUS_CACHE_DOS_HEADER_NOT_FOUND -4
#define POEDBG_STATUS_PRIVILEGES_INSUFFICIENT -3
#define POEDBG_STATUS_PRIVILEGES_NOT_ASSIGNED -2
#define POEDBG_STATUS_PRIVILEGES_NOT_FOUND -1
#define POEDBG_STATUS_SUCCESS 0

//////////////////////////////////////////////////////////////////////////
// Status Macros
//////////////////////////////////////////////////////////////////////////

#define POEDBG_SUCCESS(x) (x >= 0)
#define POEDBG_FAILURE(x) (x < 0)
#define POEDBG_RETURN_STATUS_ON_SUCCESS(f) { POEDBG_STATUS _s = f##; if (POEDBG_SUCCESS(_s)) return _s; }
#define POEDBG_RETURN_STATUS_ON_FAILURE(f) { POEDBG_STATUS _s = f##; if (POEDBG_FAILURE(_s)) return _s; }

//////////////////////////////////////////////////////////////////////////
// Configuration
//////////////////////////////////////////////////////////////////////////

#define GAME_PROCESS_NAME L"PathOfExile_x64.exe"
#define GAME_PROCESS_NAME_STEAM L"PathOfExile_x64Steam.exe"

//////////////////////////////////////////////////////////////////////////
// Macros
//////////////////////////////////////////////////////////////////////////

// Sizes.
#define DEFAULT_BUFFER_SIZE 0x100000

// Breakpoint conditions.
#define BP_CONDITION_EXECUTION 0
#define BP_CONDITION_WRITE 1
#define BP_CONDITION_READWRITE 3

// Breakpoint sizes.
#define BP_LENGTH_ONE 0
#define BP_LENGTH_TWO 1
#define BP_LENGTH_FOUR 3
#define BP_LENGTH_EIGHT 2

// Exception codes.
#define EXCEPTION_MS_VC 0x406D1388

//////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////

// Maps.
__declspec(selectany) std::map<DWORD, HANDLE> _g_GameThreads;

// Information cache about game.
__declspec(selectany) DWORD _g_GameId;
__declspec(selectany) HANDLE _g_GameHandle;
__declspec(selectany) ULONG_PTR _g_GameBaseAddress;
__declspec(selectany) ULONG_PTR _g_GameCodeCopy;
__declspec(selectany) IMAGE_DOS_HEADER _g_GameDosHeader;
__declspec(selectany) IMAGE_NT_HEADERS _g_GameNtHeaders;
__declspec(selectany) SIZE_T _g_GameImageSize;
__declspec(selectany) SIZE_T _g_GameBaseOfCode;
__declspec(selectany) SIZE_T _g_GameSizeOfCode;

// Local packet buffers.
__declspec(selectany) BYTE _g_PacketSenderBuffer[DEFAULT_BUFFER_SIZE];
__declspec(selectany) BYTE _g_PacketRecvBuffer[DEFAULT_BUFFER_SIZE];
__declspec(selectany) BYTE _g_PacketWsaRecvBuffer[DEFAULT_BUFFER_SIZE];

// Has the information cache been populated?
__declspec(selectany) bool _g_bIsGameInformationCaptured = false;

// Is the user using the Steam client?
__declspec(selectany) bool _g_bIsSteamClient = false;

//////////////////////////////////////////////////////////////////////////
// Function Declarations
//////////////////////////////////////////////////////////////////////////

DWORD __stdcall DllDebugEventHandler(LPVOID Parameter);

//////////////////////////////////////////////////////////////////////////
// API Function Pointer Types
//////////////////////////////////////////////////////////////////////////

typedef int(__stdcall *POEDBG_STANDARD_ROUTINE)();
typedef int(__stdcall *POEDBG_REGISTER_CALLBACK_ROUTINE)(PVOID Callback);

//////////////////////////////////////////////////////////////////////////
// Game Patterns
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Packet Sending Hook.

/*

POE v3.1.1e Packet Sending Function:
------------------------------------
base @ 00007ff7`2faa0000
00007ff7`2fff11df 4881ecc0040000  sub     rsp,4C0h
00007ff7`2fff11e6 48c745a0feffffff mov     qword ptr [rbp-60h],0FFFFFFFFFFFFFFFEh
00007ff7`2fff11ee 48899c24f8040000 mov     qword ptr [rsp+4F8h],rbx
00007ff7`2fff11f6 488bd9          mov     rbx,rcx
00007ff7`2fff11f9 33f6            xor     esi,esi
00007ff7`2fff11fb 89b5e0030000    mov     dword ptr [rbp+3E0h],esi
00007ff7`2fff1201 488b9188010000  mov     rdx,qword ptr [rcx+188h]
00007ff7`2fff1208 4c8b8180010000  mov     r8,qword ptr [rcx+180h]
00007ff7`2fff120f 4c2bc2          sub     r8,rdx
00007ff7`2fff1212 488b89f0000000  mov     rcx,qword ptr [rcx+0F0h]
00007ff7`2fff1219 4885c9          test    rcx,rcx
00007ff7`2fff121c 7413            je      PathOfExile_x64+0x551231 (00007ff7`2fff1231)
00007ff7`2fff121e 4d85c0          test    r8,r8
00007ff7`2fff1221 740e            je      PathOfExile_x64+0x551231 (00007ff7`2fff1231)
00007ff7`2fff1223 4c8b09          mov     r9,qword ptr [rcx]
00007ff7`2fff1226 480393a8010000  add     rdx,qword ptr [rbx+1A8h]
00007ff7`2fff122d 41ff5108        call    qword ptr [r9+8] <- Encryption function
00007ff7`2fff1231 488b8380010000  mov     rax,qword ptr [rbx+180h]
00007ff7`2fff1238 48898388010000  mov     qword ptr [rbx+188h],rax
00007ff7`2fff123f 488bfe          mov     rdi,rsi
00007ff7`2fff1242 4885c0          test    rax,rax
00007ff7`2fff1245 0f84ad040000    je      PathOfExile_x64+0x5516f8 (00007ff7`2fff16f8)
00007ff7`2fff124b 0f1f440000      nop     dword ptr [rax+rax]
00007ff7`2fff1250 488b93a8010000  mov     rdx,qword ptr [rbx+1A8h]
00007ff7`2fff1257 448b8380010000  mov     r8d,dword ptr [rbx+180h]
00007ff7`2fff125e 442bc7          sub     r8d,edi
00007ff7`2fff1261 4803d7          add     rdx,rdi
00007ff7`2fff1264 4533c9          xor     r9d,r9d
00007ff7`2fff1267 488b0b          mov     rcx,qword ptr [rbx]
00007ff7`2fff126a ff1518099000    call    qword ptr [PathOfExile_x64+0xe51b88 (00007ff7`308f1b88)] <- Call to ws2_32!send
00007ff7`2fff1270 83f8ff          cmp     eax,0FFFFFFFFh
00007ff7`2fff1273 741d            je      PathOfExile_x64+0x551292 (00007ff7`2fff1292)
00007ff7`2fff1275 4863c8          movsxd  rcx,eax

POE v3.1.1e Packet Encryption Function
--------------------------------------
rdx = buffer, r8 = length
base @ 00007ff7`2faa0000
00007ff7`302f9b70 488b4110        mov     rax, qword ptr[rcx + 10h] ds:0000022b`12f9fab0=00007ff730b03980
00007ff7`302f9b74 4883c110        add     rcx, 10h <- Hook location
00007ff7`302f9b78 4d8bc8          mov     r9, r8
00007ff7`302f9b7b 4c8bc2          mov     r8, rdx
00007ff7`302f9b7e 48ff6038        jmp     qword ptr[rax + 38h]

Signature: 48 8b 41 10 48 83 c1 10 4d 8b c8
*/

__declspec(selectany) BYTE _g_PacketSenderPattern[] =
{
	'_', 0x48, '_', 0x8b, '_', 0x41, '_', 0x10, '_', 0x48, '_', 0x83, '_', 0xc1, '_', 0x10, '_', 0x4d, '_', 0x8b, '_', 0xc8, 0x00
};

__declspec(selectany) ULONG_PTR _g_PacketSenderHookStart = NULL;
__declspec(selectany) ULONG_PTR _g_PacketSenderHookEnd = NULL;
__declspec(selectany) ULONG_PTR _g_PacketSenderHookOffset = 4;
__declspec(selectany) ULONG_PTR _g_PacketSenderHookSize = 4;

/*
POE v3.1.1e Packet Recv Function
--------------------------------
r9 = buffer, rax = length
base @ 00007ff7`2faa0000
00007ff7`2fff0d09 4889b42490000000 mov     qword ptr [rsp+90h],rsi
00007ff7`2fff0d11 488bb3c8010000  mov     rsi,qword ptr [rbx+1C8h]
00007ff7`2fff0d18 482bb3c0010000  sub     rsi,qword ptr [rbx+1C0h]
00007ff7`2fff0d1f 482bf2          sub     rsi,rdx
00007ff7`2fff0d22 48897c2468      mov     qword ptr [rsp+68h],rdi
00007ff7`2fff0d27 488b93c0010000  mov     rdx,qword ptr [rbx+1C0h]
00007ff7`2fff0d2e 483bce          cmp     rcx,rsi
00007ff7`2fff0d31 7716            ja      PathOfExile_x64+0x550d49 (00007ff7`2fff0d49)
00007ff7`2fff0d33 448bc1          mov     r8d,ecx
00007ff7`2fff0d36 4903d6          add     rdx,r14
00007ff7`2fff0d39 488b0b          mov     rcx,qword ptr [rbx]
00007ff7`2fff0d3c 4533c9          xor     r9d,r9d
00007ff7`2fff0d3f ff15630e9000    call    qword ptr [PathOfExile_x64+0xe51ba8 (00007ff7`308f1ba8)] <- Call to ws2_32!recv
00007ff7`2fff0d45 8bf8            mov     edi,eax <- Hook location
00007ff7`2fff0d47 eb78            jmp     PathOfExile_x64+0x550dc1 (00007ff7`2fff0dc1)
00007ff7`2fff0d49 4a8d0432        lea     rax,[rdx+r14]
00007ff7`2fff0d4d 4889542458      mov     qword ptr [rsp+58h],rdx
00007ff7`2fff0d52 4889442448      mov     qword ptr [rsp+48h],rax
00007ff7`2fff0d57 4c8d8c2480000000 lea     r9,[rsp+80h]
00007ff7`2fff0d5f 33c0            xor     eax,eax
00007ff7`2fff0d61 89742440        mov     dword ptr [rsp+40h],esi
00007ff7`2fff0d65 4889442430      mov     qword ptr [rsp+30h],rax
00007ff7`2fff0d6a 488d542440      lea     rdx,[rsp+40h]
00007ff7`2fff0d6f 482bce          sub     rcx,rsi
00007ff7`2fff0d72 4889442428      mov     qword ptr [rsp+28h],rax
00007ff7`2fff0d77 89842488000000  mov     dword ptr [rsp+88h],eax
00007ff7`2fff0d7e 41b802000000    mov     r8d,2

Signature: 8b f8 eb 78 4a 8d 04 32
*/

__declspec(selectany) BYTE _g_PacketRecvPattern[] =
{
	'_', 0x8b, '_', 0xf8, '_', 0xeb, '_', 0x78, '_', 0x4a, '_', 0x8d, '_', 0x04, '_', 0x32, 0x00
};

__declspec(selectany) ULONG_PTR _g_PacketRecvHookStart = NULL;
__declspec(selectany) ULONG_PTR _g_PacketRecvHookEnd = NULL;
__declspec(selectany) ULONG_PTR _g_PacketRecvHookOffset = 0;
__declspec(selectany) ULONG_PTR _g_PacketRecvHookSize = 2;

/*
POE v3.1.1e Packet WSARecv Function
-----------------------------------
Explanation below
base @ 00007ff7`2faa0000
00007ff7`2fff0e01 488b93c0010000  mov     rdx,qword ptr [rbx+1C0h]
00007ff7`2fff0e08 4c8b09          mov     r9,qword ptr [rcx]
00007ff7`2fff0e0b 4903d6          add     rdx,r14
00007ff7`2fff0e0e 8bc7            mov     eax,edi
00007ff7`2fff0e10 483bc6          cmp     rax,rsi
00007ff7`2fff0e13 7709            ja      PathOfExile_x64+0x550e1e (00007ff7`2fff0e1e)
00007ff7`2fff0e15 4c63c7          movsxd  r8,edi
00007ff7`2fff0e18 41ff5110        call    qword ptr [r9+10h]
00007ff7`2fff0e1c eb21            jmp     PathOfExile_x64+0x550e3f (00007ff7`2fff0e3f)
00007ff7`2fff0e1e 4c8bc6          mov     r8,rsi
00007ff7`2fff0e21 41ff5110        call    qword ptr [r9+10h] <- Decryption function?
00007ff7`2fff0e25 488b8bf0000000  mov     rcx,qword ptr [rbx+0F0h]
00007ff7`2fff0e2c 488b93c0010000  mov     rdx,qword ptr [rbx+1C0h]
00007ff7`2fff0e33 4c63c7          movsxd  r8,edi
00007ff7`2fff0e36 4c2bc6          sub     r8,rsi
00007ff7`2fff0e39 488b01          mov     rax,qword ptr [rcx]
00007ff7`2fff0e3c ff5010          call    qword ptr [rax+10h] <- Decryption function?
00007ff7`2fff0e3f 4863c7          movsxd  rax,edi <- Hook location
00007ff7`2fff0e42 48018398010000  add     qword ptr [rbx+198h],rax
00007ff7`2fff0e49 488b83e8000000  mov     rax,qword ptr [rbx+0E8h]
00007ff7`2fff0e50 017804          add     dword ptr [rax+4],edi
00007ff7`2fff0e53 488b83c8010000  mov     rax,qword ptr [rbx+1C8h]
00007ff7`2fff0e5a 482b83c0010000  sub     rax,qword ptr [rbx+1C0h]
00007ff7`2fff0e61 48398398010000  cmp     qword ptr [rbx+198h],rax
00007ff7`2fff0e68 7519            jne     PathOfExile_x64+0x550e83 (00007ff7`2fff0e83)
00007ff7`2fff0e6a 488b93c8010000  mov     rdx,qword ptr [rbx+1C8h]
00007ff7`2fff0e71 488bcb          mov     rcx,rbx
00007ff7`2fff0e74 482b93c0010000  sub     rdx,qword ptr [rbx+1C0h]
00007ff7`2fff0e7b 4803d2          add     rdx,rdx
00007ff7`2fff0e7e e88d0a0000      call    PathOfExile_x64+0x551910 (00007ff7`2fff1910)
00007ff7`2fff0e83 488b7c2468      mov     rdi,qword ptr [rsp+68h]
00007ff7`2fff0e88 488bb42490000000 mov     rsi,qword ptr [rsp+90h]

Notes:

- At hook location for a 2-buffer setup, RSP looks as follows:
000000d3`5a8feee0  00000233`00000040 00000000`00000000
000000d3`5a8feef0  00000000`00000742 00000000`00000009
000000d3`5a8fef00  000000d3`5a8fef68 00000000`00000000
000000d3`5a8fef10  00000000`00000000 00000000`00000001
000000d3`5a8fef20  00000000`00000742 00000233`ab004f7e <- Buffer 1 size, buffer 1 pointer
000000d3`5a8fef30  00000000`000000be 00000233`ab004ec0 <- Buffer 2 size, buffer 2 pointer
000000d3`5a8fef40  ffffffff`ffffffff 00000233`406c5bf0
000000d3`5a8fef50  00000000`0e8f55af 00007ff7`3031fea4

- At hook location, RDI contains actual bytes received.
- Can reach first buffer size at RSP+0x40, buffer location at RSP+0x48.
- Can keep reading QWORDS until 0xffffffff`ffffffff.
- If RDI is greater than the size of buffer 1, must also read from buffer 2, etc.

Signature: 48 63 c7 48 01 83 98 01 00 00
*/

__declspec(selectany) BYTE _g_PacketWsaRecvPattern[] =
{
	'_', 0x48, '_', 0x63, '_', 0xc7, '_', 0x48, '_', 0x01, '_', 0x83, '_', 0x98, '_', 0x01, '_', 0x00, '_', 0x00, 0x00
};

__declspec(selectany) ULONG_PTR _g_PacketWsaRecvHookStart = NULL;
__declspec(selectany) ULONG_PTR _g_PacketWsaRecvHookEnd = NULL;
__declspec(selectany) ULONG_PTR _g_PacketWsaRecvHookOffset = 0;
__declspec(selectany) ULONG_PTR _g_PacketWsaRecvHookSize = 3;
