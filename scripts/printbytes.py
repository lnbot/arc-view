#!/usr/bin/env python3

import sys

n = int(sys.argv[1])
byte_len = 4;

# Convert to little‑endian bytes
b = n.to_bytes(byte_len, byteorder="little")

# Print as hex with no spaces
print(b.hex())
