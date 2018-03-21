# Part of 'poedbg'. Copyright (c) 2018 maper. Copies must retain this attribution.

import ctypes
import time
import sys

# First, we need to specify the types of our callbacks so that the ctypes
# module knows what to do with them when we pass them to poedbg.

@ctypes.WINFUNCTYPE(None, ctypes.c_int)
def poedbg_error_callback(error_code):
    print("[ERROR] The 'poedbg' module reported an error code of '{}'.".format(error_code))

@ctypes.WINFUNCTYPE(None, ctypes.c_int, ctypes.c_byte, ctypes.c_void_p)
def poedbg_packet_receive_callback(packet_length, packet_id, packet_data):
    print("[RECEIVED] Packet with ID of '{}' and length of '{}'.".format(packet_id, packet_length))

    data = ctypes.cast(packet_data, ctypes.POINTER(ctypes.c_char * packet_length))
    for b in data.contents:
        sys.stdout.write("{:02x} ".format(ord(b)))
    print("")

@ctypes.WINFUNCTYPE(None, ctypes.c_int, ctypes.c_byte, ctypes.c_void_p)
def poedbg_packet_send_callback(packet_length, packet_id, packet_data):
    print("[SENT] Packet with ID of '{}' and length of '{}'.".format(packet_id, packet_length))
    
    data = ctypes.cast(packet_data, ctypes.POINTER(ctypes.c_char * packet_length))
    for b in data.contents:
        sys.stdout.write("{:02x} ".format(ord(b)))
    print("")


def main():
    print("Starting 'poedbg' Python sample...")

    # Load an instance of poedbg.
    poedbg_dll = ctypes.windll.LoadLibrary('poedbg.dll')

    # Next, we can directly call all of the poedbg functions to set up
    # our callbacks. We should make sure that we set up the error handler
    # especially before initializing the module later.

    poedbg_status = poedbg_dll.PoeDbgRegisterErrorCallback(poedbg_error_callback)
    if poedbg_status < 0:
        print("[ERROR] Could not register error callback. Error code: '{}'.".format(poedbg_status))

    poedbg_status = poedbg_dll.PoeDbgRegisterPacketReceiveCallback(poedbg_packet_receive_callback)
    if poedbg_status < 0:
        print("[ERROR] Could not register packet receive callback. Error code: '{}'.".format(poedbg_status))

    poedbg_status = poedbg_dll.PoeDbgRegisterPacketSendCallback(poedbg_packet_send_callback)
    if poedbg_status < 0:
        print("[ERROR] Could not register packet send callback. Error code: '{}'.".format(poedbg_status))

    poedbg_status = poedbg_dll.PoeDbgInitialize()
    if poedbg_status < 0:
        print("[ERROR] Could not intitialize. Error code: '{0}'.".format(poedbg_status))

    print("Started successfully.")

    # If we let the main thread die, our game hooks will be un-set. This
    # will crash the game.

    raw_input("(Pressing a key will disable poedbg)...")

    poedbg_status = poedbg_dll.PoeDbgDestroy()
    if poedbg_status < 0:
        print("[ERROR] Could not destroy. Error code: '{0}'.".format(poedbg_status))

    print("Removed successfully.")

    raw_input("(Pressing a key will quit)...")

if __name__ == "__main__":
    main()