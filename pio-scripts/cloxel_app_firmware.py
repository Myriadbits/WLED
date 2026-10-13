Import('env')
import os
import shutil
import hashlib
import json

# Copies the built firmware into the Cloxel app so it can be flashed over BLE.
# Enable per environment with:
#   extra_scripts = ${scripts_defaults.extra_scripts} post:pio-scripts/cloxel_app_firmware.py
#   custom_cloxel_app_firmware_dir = ../CloxelApp/Resources/Raw/firmware
# The version is taken from -D CLOXEL_VERSION=\"x.y.z\" (also reported by the device over BLE).

FIRMWARE_BIN = "cloxel_firmware.bin"
FIRMWARE_MANIFEST = "cloxel_firmware.json"

def _get_cpp_define_value(env, define):
    define_list = [item[-1] for item in env["CPPDEFINES"] if isinstance(item, (list, tuple)) and item[0] == define]
    if define_list:
        return str(define_list[0]).replace("\\\"", "").replace("\"", "")
    return None

def copy_firmware_to_app(source, target, env):
    target_dir = env.GetProjectOption("custom_cloxel_app_firmware_dir", "")
    if not target_dir:
        print("Cloxel app firmware: custom_cloxel_app_firmware_dir not set, skipping")
        return
    target_dir = os.path.normpath(os.path.join(env["PROJECT_DIR"], target_dir))

    version = _get_cpp_define_value(env, "CLOXEL_VERSION")
    if not version:
        print("Cloxel app firmware: CLOXEL_VERSION not defined, skipping")
        return

    bin_path = str(target[0])
    with open(bin_path, "rb") as f:
        data = f.read()

    os.makedirs(target_dir, exist_ok=True)
    shutil.copy(bin_path, os.path.join(target_dir, FIRMWARE_BIN))
    manifest = {
        "version": version,
        "release": _get_cpp_define_value(env, "WLED_RELEASE_NAME") or "",
        "size": len(data),
        "sha256": hashlib.sha256(data).hexdigest(),
        "file": FIRMWARE_BIN,
    }
    with open(os.path.join(target_dir, FIRMWARE_MANIFEST), "w") as f:
        json.dump(manifest, f, indent=2)
    print(f"Cloxel app firmware: copied version {version} ({len(data)} bytes) to {target_dir}")

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", copy_firmware_to_app)
