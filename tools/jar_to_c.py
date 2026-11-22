#!/usr/bin/env python3
"""
Convert any J2ME MIDP 2.0 .jar file into a C byte array for embedded flashing on ESP32-S3.

Usage:
    python jar_to_c.py <input_game.jar> <output_c_file.c> [array_name]

Example:
    python jar_to_c.py Gangstar_Crime_City_2006.jar gangstar_jar.c gangstar_jar
"""

import sys
import os

def jar_to_c(jar_path, c_path, var_name="gangstar_jar"):
    if not os.path.exists(jar_path):
        print(f"Error: {jar_path} does not exist.")
        sys.exit(1)

    with open(jar_path, "rb") as f:
        data = f.read()

    size = len(data)
    print(f"Read {size} bytes from {jar_path}")

    with open(c_path, "w", encoding="utf-8") as out:
        out.write("#include <stdint.h>\n\n")
        out.write(f"// Embedded J2ME JAR: {os.path.basename(jar_path)} ({size} bytes)\n")
        out.write(f"const uint32_t {var_name}_len = {size};\n")
        out.write(f"const uint8_t {var_name}[] = {{\n")

        for i in range(0, size, 16):
            chunk = data[i:i+16]
            hex_str = ", ".join(f"0x{b:02X}" for b in chunk)
            if i + 16 < size:
                out.write(f"    {hex_str},\n")
            else:
                out.write(f"    {hex_str}\n")

        out.write("};\n")

    print(f"Successfully generated {c_path} ({os.path.getsize(c_path)} bytes)")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)

    jar = sys.argv[1]
    c_out = sys.argv[2]
    name = sys.argv[3] if len(sys.argv) > 3 else "gangstar_jar"
    jar_to_c(jar, c_out, name)
