import os
import re
import glob

print("Patching library conflicts with ESP32 core...")

# List of conflicts: (file_pattern, search_pattern, replace_pattern)
conflicts = [
    (
        ".pio/libdeps/*/ESP8266SAM/src/render.c",
        r'static void yield\(\)',
        'static void sam_yield()'
    ),
    (
        ".pio/libdeps/*/ESP8266SAM/src/render.c",
        r'\byield\(\);',
        'sam_yield();'
    ),
    (
        ".pio/libdeps/*/JPEGDecoder/src/picojpeg.c",
        r'static uint8 init\(void\)',
        'static uint8 picojpeg_init(void)'
    ),
    (
        ".pio/libdeps/*/JPEGDecoder/src/picojpeg.c",
        r'\binit\(\);',
        'picojpeg_init();'
    ),
    (
        ".pio/libdeps/*/ESP Amiibolink/src/amiibolink.h",
        r'#include <NimBLEDevice.h>',
        '#include <Arduino.h>\n#include <NimBLEDevice.h>'
    ),
    (
        ".pio/libdeps/*/ESP Chameleon Ultra/src/chameleonUltra.h",
        r'#include <NimBLEDevice.h>',
        '#include <Arduino.h>\n#include <NimBLEDevice.h>'
    ),
    (
        ".pio/libdeps/*/ESP PN32BLE/src/pn532_ble.h",
        r'#include <NimBLEDevice.h>',
        '#include <Arduino.h>\n#include <NimBLEDevice.h>'
    ),
    (
        ".pio/libdeps/*/LovyanGFX/src/lgfx/v1/panel/Panel_SSD1306.cpp",
        r'bool flg = 256 <= value \+ Bayer\[\(x \+ \(y << 2\)\) & 31\];',
        'bool flg = value > 0;'
    ),
    (
        ".pio/libdeps/*/LovyanGFX/src/lgfx/v1/panel/Panel_SSD1306.cpp",
        r'bool flg = 256 <= value \+ btbl\[\(x \+ _bayer_offset\) & 3\];',
        'bool flg = value > 0;'
    ),
]

for file_pattern, search, replace in conflicts:
    files = glob.glob(file_pattern)
    for file_path in files:
        try:
            with open(file_path, 'r') as f:
                content = f.read()

            if replace in content:
                print(f"Already patched: {file_path}")
                continue

            new_content = re.sub(search, replace, content)
            if new_content != content:
                with open(file_path, 'w') as f:
                    f.write(new_content)
                print(f"Patched: {file_path}")
        except Exception as e:
            print(f"Failed to patch {file_path}: {e}")
