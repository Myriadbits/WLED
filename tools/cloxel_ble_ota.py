#!/usr/bin/env python3
"""
Flash a Cloxel over BLE from a PC (test tool for the BLEOta protocol, see
usermods/wordcloxel/BLEConfig/BLEOta.h).

    pip install bleak
    python tools/cloxel_ble_ota.py                       # scan for Cloxel devices
    python tools/cloxel_ble_ota.py <address> firmware.bin
"""
import asyncio
import hashlib
import struct
import sys

from bleak import BleakClient, BleakScanner

CONTROL_UUID = "ebd7f0a1-04a0-4f9c-96f3-05644d494f54"
DATA_UUID = "ebd7f0a2-04a0-4f9c-96f3-05644d494f54"

CMD_START, CMD_FINISH, CMD_ABORT = 0x01, 0x02, 0x03
RSP_READY, RSP_ACK, RSP_DONE, RSP_ERROR = 0x81, 0x82, 0x83, 0x84
ERR_OFFSET = 4


async def scan():
    for device in await BleakScanner.discover(timeout=5):
        if device.name and "cloxel" in device.name.lower():
            print(f"{device.address}  {device.name}")


async def flash(address, path):
    image = open(path, "rb").read()
    responses = asyncio.Queue()

    async with BleakClient(address, timeout=20) as client:
        chunk_size = min(client.mtu_size - 3, 512) - 4
        print(f"Connected, MTU {client.mtu_size}, chunk size {chunk_size}, image {len(image)} bytes")
        await client.start_notify(CONTROL_UUID, lambda _, data: responses.put_nowait(bytes(data)))

        async def response(timeout):
            return await asyncio.wait_for(responses.get(), timeout)

        await client.write_gatt_char(CONTROL_UUID, struct.pack("<BI", CMD_START, len(image)) + hashlib.sha256(image).digest(), response=True)
        rsp = await response(15)
        if rsp[0] != RSP_READY:
            raise RuntimeError(f"START failed: {rsp.hex()}")
        window = struct.unpack_from("<H", rsp, 1)[0]

        try:
            offset = 0
            while offset < len(image):
                window_end = min(len(image), (offset // window + 1) * window)
                for pos in range(offset, window_end, chunk_size):
                    chunk = image[pos:min(pos + chunk_size, window_end)]
                    await client.write_gatt_char(DATA_UUID, struct.pack("<I", pos) + chunk, response=False)
                rsp = await response(10)
                if rsp[0] == RSP_ACK:
                    offset = struct.unpack_from("<I", rsp, 1)[0]
                elif rsp[0] == RSP_ERROR and rsp[1] == ERR_OFFSET:
                    offset = struct.unpack_from("<I", rsp, 2)[0]
                    print(f"\nResending from {offset}")
                else:
                    raise RuntimeError(f"Transfer failed: {rsp.hex()}")
                print(f"\r{offset * 100 // len(image)}%", end="", flush=True)

            await client.write_gatt_char(CONTROL_UUID, bytes([CMD_FINISH]), response=True)
            rsp = await response(15)
            if rsp[0] != RSP_DONE:
                raise RuntimeError(f"FINISH failed: {rsp.hex()}")
            print("\nDone, the device restarts")
        except BaseException:
            await client.write_gatt_char(CONTROL_UUID, bytes([CMD_ABORT]), response=True)
            raise


if __name__ == "__main__":
    if len(sys.argv) == 3:
        asyncio.run(flash(sys.argv[1], sys.argv[2]))
    else:
        asyncio.run(scan())
