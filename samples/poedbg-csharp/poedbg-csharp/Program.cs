// Part of 'poedbg'. Copyright (c) 2018 maper. Copies must retain this attribution.

using System;
using System.Runtime.InteropServices;

namespace poedbg_csharp
{
    class Program
    {
        // First we need to define types for the callbacks that we're going
        // to register with poedbg.

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate void PoeDbgErrorCallback(int ErrorCode);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate void PoeDbgPacketSendCallback(int Length, byte Id, byte[] Data);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate void PoeDbgPacketReceiveCallback(int Length, byte Id, byte[] Data);

        // Next we need to reference all of the functions we will import
        // from poedbg.

        [DllImport("poedbg.dll")]
        public static extern int PoeDbgInitialize();

        [DllImport("poedbg.dll")]
        public static extern int PoeDbgRegisterErrorCallback([MarshalAs(UnmanagedType.FunctionPtr)] PoeDbgErrorCallback Callback);

        [DllImport("poedbg.dll")]
        public static extern int PoeDbgRegisterPacketSendCallback([MarshalAs(UnmanagedType.FunctionPtr)] PoeDbgPacketSendCallback Callback);

        [DllImport("poedbg.dll")]
        public static extern int PoeDbgRegisterPacketReceiveCallback([MarshalAs(UnmanagedType.FunctionPtr)] PoeDbgPacketReceiveCallback Callback);

        [DllImport("poedbg.dll")]
        public static extern int PoeDbgUnregisterErrorCallback();

        [DllImport("poedbg.dll")]
        public static extern int PoeDbgUnregisterPacketSendCallback();

        [DllImport("poedbg.dll")]
        public static extern int PoeDbgUnregisterPacketReceiveCallback();

        static void Main(string[] args)
        {
            Console.WriteLine("Starting 'poedbg' C# sample...");

            // We'll define the callback handlers here as anonymous functions just
            // for the sake of example.

            PoeDbgErrorCallback ErrorCallback =
            (ErrorCode) =>
            {
                Console.WriteLine("[ERROR] The 'poedbg' module reported an error code of '{0}'.", ErrorCode);
            };

            PoeDbgPacketSendCallback SendCallback =
            (Length, Id, Data) =>
            {
                Console.WriteLine("[SENT] Packet with ID of '{0}' and length of '{1}'.", Id, Length);
            };

            PoeDbgPacketReceiveCallback ReceiveCallback =
            (Length, Id, Data) =>
            {
                Console.WriteLine("[RECEIVED] Packet with ID of '{0}' and length of '{1}'.", Id, Length);
            };

            // Now we'll register all of our callbacks. It is important, especially,
            // that we register the error handling callback before we attempt to
            // initialize the poedbg module.

            int Status = PoeDbgRegisterErrorCallback(ErrorCallback);
            if (Status < 0)
            {
                Console.WriteLine("[ERROR] Could not register error callback. Error code: '{0}'.", Status);
            }

            Status = PoeDbgRegisterPacketSendCallback(SendCallback);
            if (Status < 0)
            {
                Console.WriteLine("[ERROR] Could not register packet send callback. Error code: '{0}'.", Status);
            }

            Status = PoeDbgRegisterPacketReceiveCallback(ReceiveCallback);
            if (Status < 0)
            {
                Console.WriteLine("[ERROR] Could not register packet receive callback. Error code: '{0}'.", Status);
            }

            Status = PoeDbgInitialize();
            if (Status < 0)
            {
                Console.WriteLine("[ERROR] Could not initialize. Error code: '{0}'.", Status);
            }

            Console.WriteLine("Started successfully.");

            // Wait.
            Console.ReadKey(true);
        }
    }
}
