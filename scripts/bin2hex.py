#!/usr/bin/env python3

import sys

if len(sys.argv) !=3:
    print("Usage: python3 scripts/bin2hex.py <input.bin> <output.bin>")
    sys.exit(1)

input_file = sys.argv[1] 
output_file = sys.argv[2]

with open(input_file, "rb") as f:
    data = f.read() 

with open(output_file, "w") as f:
    for i in range(0, len(data), 4):
        word = int.from_bytes(data[i:i+4], "little")
        f.write(f"{word:08x}\n")

print(f"Created {output_file} with {len(data)//4} instructions")