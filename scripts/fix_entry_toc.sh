#!/bin/bash
# fix_entry_toc.sh - Patches the ELF entry point to include the correct TOC value
#
# PSL1GHT's lv2-crt0.o defines _start in .rodata with the entry point address
# duplicated in both the addr and rtoc fields of the entry descriptor.
# RPCS3 reads the entry point as a 32-bit OPD (ppu_func_opd_t = {addr, rtoc}),
# so the rtoc field must contain the TOC base, not a copy of the code address.
#
# This script:
#   1. Reads the TOC value from the __start OPD in the .opd section
#   2. Patches the second 32-bit word at the _start entry in .rodata with the TOC
#
# Usage: fix_entry_toc.sh <elf_file>

set -e

ELF="$1"

if [ -z "$ELF" ] || [ ! -f "$ELF" ]; then
    echo "Error: ELF file not found: $ELF" >&2
    exit 1
fi

# Get the addresses of key symbols
START_CODE=$(ppu-nm "$ELF" | grep " T \._start$" | awk '{print $1}')
ENTRY_RODATA=$(ppu-nm "$ELF" | grep " R _start$" | awk '{print $1}')
OPD_START=$(ppu-nm "$ELF" | grep " [td] __start$" | awk '{print $1}')

if [ -z "$START_CODE" ] || [ -z "$ENTRY_RODATA" ] || [ -z "$OPD_START" ]; then
    echo "Warning: Could not find required symbols, skipping patch" >&2
    exit 0
fi

# Convert hex addresses to decimal (strip leading zeros)
ENTRY_VA=$((16#${ENTRY_RODATA}))
OPD_VA=$((16#${OPD_START}))

# Find the file offsets using readelf program headers
# Parse LOAD segments to find the mapping: file_offset = vaddr - segment_vaddr + segment_offset
get_file_offset() {
    local va=$1
    ppu-readelf -lW "$ELF" 2>/dev/null | grep "LOAD" | while read -r type offset vaddr phaddr fsize msize flags align; do
        # Remove 0x prefix and convert
        local seg_offset=$((16#${offset#0x}))
        local seg_vaddr=$((16#${vaddr#0x}))
        local seg_fsize=$((16#${fsize#0x}))
        local seg_end=$((seg_vaddr + seg_fsize))
        if [ "$va" -ge "$seg_vaddr" ] && [ "$va" -lt "$seg_end" ]; then
            echo $((va - seg_vaddr + seg_offset))
            return
        fi
    done
}

# Calculate file offsets
ENTRY_FILE_OFF=$(get_file_offset $ENTRY_VA)
OPD_FILE_OFF=$(get_file_offset $OPD_VA)

if [ -z "$ENTRY_FILE_OFF" ] || [ -z "$OPD_FILE_OFF" ]; then
    echo "Warning: Could not determine file offsets, skipping patch" >&2
    exit 0
fi

# Read the TOC value from __start OPD
# The OPD is in 64-bit format: [addr64, toc64]
# TOC is at OPD + 8 (as 64-bit), its low 32 bits are at OPD + 12
TOC_OFF=$((OPD_FILE_OFF + 12))
TOC_HEX=$(dd if="$ELF" bs=1 skip=$TOC_OFF count=4 2>/dev/null | od -A n -t x1 | tr -d ' \n')

if [ -z "$TOC_HEX" ] || [ "$TOC_HEX" = "00000000" ]; then
    echo "Warning: TOC value is zero or unreadable, skipping patch" >&2
    exit 0
fi

# Verify the current entry data (first 4 bytes should be the code address)
ENTRY_ADDR_HEX=$(dd if="$ELF" bs=1 skip=$ENTRY_FILE_OFF count=4 2>/dev/null | od -A n -t x1 | tr -d ' \n')
ENTRY_RTOC_HEX=$(dd if="$ELF" bs=1 skip=$((ENTRY_FILE_OFF + 4)) count=4 2>/dev/null | od -A n -t x1 | tr -d ' \n')

# Only patch if the rtoc field contains the code address (the known bug)
if [ "$ENTRY_ADDR_HEX" = "$ENTRY_RTOC_HEX" ]; then
    # Patch: write TOC value at entry + 4 (the rtoc field)
    printf "\\x${TOC_HEX:0:2}\\x${TOC_HEX:2:2}\\x${TOC_HEX:4:2}\\x${TOC_HEX:6:2}" | \
        dd of="$ELF" bs=1 seek=$((ENTRY_FILE_OFF + 4)) conv=notrunc 2>/dev/null
    echo "  Entry point patched: addr=0x$ENTRY_ADDR_HEX, rtoc=0x$TOC_HEX (was 0x$ENTRY_RTOC_HEX)"
else
    echo "  Entry point already correct: addr=0x$ENTRY_ADDR_HEX, rtoc=0x$ENTRY_RTOC_HEX"
fi
