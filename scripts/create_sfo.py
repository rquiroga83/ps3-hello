#!/usr/bin/env python3
import struct
import sys

if len(sys.argv) < 3:
    print("Usage: create_sfo.py <title> <appid> <output_file>")
    sys.exit(1)

title = sys.argv[1]
appid = sys.argv[2]
output = sys.argv[3]

MAGIC = 0x46535000
VERSION = 0x00000101
TYPE_UTF8 = 0x0004
TYPE_UTF8_SPECIAL = 0x0204
TYPE_INT32 = 0x0404

def align(val, alignment):
    return ((val + alignment - 1) // alignment) * alignment

entries = [
    ("APP_VER", TYPE_UTF8, "01.00"),
    ("ATTRIBUTE", TYPE_INT32, 0),
    ("BOOTABLE", TYPE_INT32, 1),
    ("CATEGORY", TYPE_UTF8_SPECIAL, "HG"),
    ("LICENSE", TYPE_UTF8, "Copyright Notice"),
    ("PARENTAL_LEVEL", TYPE_INT32, 0),
    ("PS3_SYSTEM_VER", TYPE_UTF8, "04.8800"),
    ("RESOLUTION", TYPE_INT32, 63),
    ("SOUND_FORMAT", TYPE_INT32, 1),
    ("TITLE", TYPE_UTF8_SPECIAL, title),
    ("TITLE_ID", TYPE_UTF8, appid),
    ("VERSION", TYPE_UTF8, "01.00"),
]

entries.sort(key=lambda x: x[0])

header_size = 0x14
entry_count = len(entries)
index_table_size = entry_count * 0x10
key_table_offset = header_size + index_table_size

key_table = b""
for key, _, _ in entries:
    key_table += key.encode('utf-8') + b'\x00'

data_table_offset = key_table_offset + len(key_table)

data_table = b""
index_table = b""
key_offset = 0

for key, data_type, value in entries:
    data_offset = len(data_table)
    
    if data_type == TYPE_INT32:
        data_size = 4
        data_max_size = 4
        data_bytes = struct.pack('<I', value)
    else:
        value_bytes = value.encode('utf-8') + b'\x00'
        data_size = len(value_bytes)
        data_max_size = align(data_size, 4)
        data_bytes = value_bytes + b'\x00' * (data_max_size - data_size)
    
    index_table += struct.pack('<HH', key_offset, data_type)
    index_table += struct.pack('<III', data_size, data_max_size, data_offset)
    data_table += data_bytes
    key_offset += len(key.encode('utf-8')) + 1

with open(output, 'wb') as f:
    f.write(struct.pack('<I', MAGIC))
    f.write(struct.pack('<I', VERSION))
    f.write(struct.pack('<I', key_table_offset))
    f.write(struct.pack('<I', data_table_offset))
    f.write(struct.pack('<I', entry_count))
    f.write(index_table)
    f.write(key_table)
    f.write(data_table)
