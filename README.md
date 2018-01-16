![poedbg](misc/images/logo.jpg?raw=true "poedbg")
---

A simple API to interact with the [Path of Exile](http://www.pathofexile.com) game client for tasks like packet logging, memory manipulation, and more. Even better - all in your favorite languages. This project continues to be a work in progress.

While this software is not intended to provide any explicit competitive advantage, it may still be against the game's terms of service. Please use it responsibly, and at your own risk.

### Supported Features and Languages

At the moment, **C++**, **C#**, and **Python** are supported first-class languages.

The library currently supports these features:
* Packet receive notifications.
* Packet send notifications.

### Requirements

The _poedbg_ library currently only works on the Windows operating system, and is only compatible with the 64-bit version of the game. Both the standard and Steam versions are supported.

### Getting Started

#### Library

The latest compiled version of the library [can be found here](https://github.com/m4p3r/poedbg/blob/master/bin/poedbg.dll).

#### C++ and C#

You can find the C++ [sample code here](https://github.com/m4p3r/poedbg/tree/master/samples/poedbg-cpp).

You can find the C# [sample code here](https://github.com/m4p3r/poedbg/tree/master/samples/poedbg-csharp).

For both of these samples, make sure that you build the project for the x64 architecture. Once built, simply make sure the latest _poedbg.dll_ is in the same folder as the new executable. Run the executable as administrator.

#### Python

You can find the Python [sample code here](https://github.com/m4p3r/poedbg/tree/master/samples/poedbg-python).

You must make sure that you are using the 64-bit Python interpreter when running the script, or it will not correctly load _poedbg.dll_. Make sure that you run the console as administrator before executing the script. Also ensure that the latest _poedbg.dll_ is in the same folder as the script.

### Status Codes

Most of the exported APIs in _poedbg_ will return a status code. Positive status codes (>= 0) indicate success, while negative status codes (< 0) indicate failure. For detailed error information, refer to this table.

Value | Name | Description
--- | --- | ---
0 | `POEDBG_STATUS_SUCCESS` | The operation completed successfully.
-1 | `POEDBG_STATUS_PRIVILEGES_NOT_FOUND` | The privilege value required for the host application was not found on this computer. You may be running as a user with restricted privileges.
-2 | `POEDBG_STATUS_PRIVILEGES_NOT_ASSIGNED` | The privilege value required for the host application was not able to be assigned. You may not have sufficient privileges to apply the required value.
-3 | `POEDBG_STATUS_PRIVILEGES_INSUFFICIENT` | The user is not running as administrator.
-4 | `POEDBG_STATUS_CACHE_DOS_HEADER_NOT_FOUND` | The library was unable to locate a PE DOS header for the game.
-5 | `POEDBG_STATUS_CACHE_NT_HEADER_NOT_FOUND` | The library was unable to locate a PE NT header for the game.
-6 | `POEDBG_STATUS_CACHE_NT_HEADER_INVALID` | The NT header located was not valid.
-7 | `POEDBG_STATUS_CACHE_ALLOCATION_FAILED` | The library was unable to cache sufficient memory in the host application. There may be memory pressure in your application that is preventing the library from caching.
-8 | `POEDBG_STATUS_CACHE_COPY_FAILED` | The library was unable to cache data from the game into the host application. There may be a permissions problem with your application.
-9 | `POEDBG_STATUS_HOOK_SEND_FAILED` | The game's send() hook location was not found or could not be applied. This could be due to a game update or running an altered version of the game.
-10 | `POEDBG_STATUS_HOOK_RECV_FAILED` | The game's recv() hook location was not found or could not be applied. This could be due to a game update or running an altered version of the game.
-11 | `POEDBG_STATUS_HOOK_WSARECV_FAILED` | The game's WSArecv() hook location was not found or could not be applied. This could be due to a game update or running an altered version of the game.
-12 | `POEDBG_STATUS_GAME_NOT_FOUND` | The game is not running.
-13 | `POEDBG_STATUS_GAME_HOOK_NOT_SET` | The debugging engine could not be attached to the game.
-14 | `POEDBG_STATUS_GAME_HOOK_BEHAVIOR_NOT_SET` | The debugger behavior could not be adjusted.
-15 | `POEDBG_STATUS_CALLBACK_NOT_SUPPORTED` | The provided callback is not supported.
-16 | `POEDBG_STATUS_CALLBACK_ALREADY_REGISTERED` | The provided callback is already registered.
-17 | `POEDBG_STATUS_EXCEPTION_NOT_HANDLED` | The library detected a second-chance unhandled exception and the game can not recover.

### Donations

I mainly do this for fun, but if you'd like to show your appreciation via a small donation, you're welcome to send donations to any of these wallet addresses:

```
Bitcoin: 18fMKKzSZYJ6JGfrdUQZbw5AmY7qjADu6h
Bitcoin Cash: 19mZN2hLNTjhgM4oF9iAhRjZmcu7mrZvuH
Ethereum: 0xDD86F0C5Db3f5FecCB5850Deed6d9b7c27064c5b
Litecoin: LgzXydJ2DDxzkBXpMHRpvRbRqve7MC2foZ
```

### License

You are free to use this software as you see fit. Please keep in mind that some uses of this software may be a violation of the game's terms of service, which I am not condoning.

If you do use this software, please retain the attribution to _poedbg_ in your source files, and consider open sourcing your own work to help the community learn and grow.
