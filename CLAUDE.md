# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

PS3 homebrew application targeting the Cell Broadband Engine, built with the [PSL1GHT](https://github.com/ps3dev/PSL1GHT) open-source SDK and [ps3toolchain](https://github.com/ps3dev/ps3toolchain) cross-compiler. The project demonstrates SPE (Synergistic Processing Element) usage for parallel SIMD vector calculations.

## Build Commands

All compilation must happen inside the Dev Container (provides `ppu-lv2-gcc`, `spu-lv2-gcc`, etc.).

```bash
make          # Build SPU program first, then PPU (produces .elf and .self)
make spu      # Build only the SPU program (generates data/spu.bin)
make clean    # Remove all build artifacts
make pkg      # Build and package as installable .pkg
```

Output files: `hola-ps3.elf`, `hola-ps3.self` (signed, PS3-ready), `data/spu.bin` (SPU binary).

There is no test framework. Test manually via [RPCS3 emulator](https://rpcs3.net/) or PS3 hardware (CFW/HEN).

## Architecture

Two-stage cross-compilation matching the Cell BE's heterogeneous processor design:

1. **SPU code** (`spu/source/main.c`) compiles via `spu_rules` → `spu.elf` → copied to `data/spu.bin`
2. **PPU code** (`source/main.c`) compiles via `ppu_rules`, embedding `data/spu.bin` via `bin2o` (binary-to-object)

**Shared data:** `include/vecmath.h` defines `vecmath_data_t`, a 128-byte aligned struct used for PPU↔SPU DMA transfers. PPU↔SPU communication is exclusively through DMA — the struct must stay **128-byte aligned** (`__attribute__((aligned(128)))`).

### Key constraints

- SPU local store is **256KB** — all SPU code + data must fit within this limit
- SIMD operations work on 128-bit vectors (4x float) natively via `spu_*` intrinsics
- `PSL1GHT` environment variable must be set (handled by the Dev Container)

## Development Environment

Use VS Code Dev Containers. The Docker image is `flipacholas/ps3devextra:latest` (pre-built with ps3toolchain):

```bash
# In VS Code: "Dev Containers: Reopen in Container"
```

The container provides environment variables `PS3DEV=/usr/local/ps3dev` and `PSL1GHT=${PS3DEV}`.

## Build System Details

The root `Makefile` orchestrates the two-stage build:
- `make all` runs `make -C spu` then `make -C spu install` (copies `spu.elf` → `data/spu.bin`), then builds the PPU ELF
- The PPU build uses `bin2o` to convert `data/spu.bin` into a linkable object, exposing `spu_bin[]` and `spu_bin_size` symbols in C
- PPU links against `-lrt -llv2 -lm`; SPU has no library dependencies

## PPU↔SPU Communication Pattern

1. PPU allocates a 128-byte aligned `vecmath_data_t` struct and fills input fields
2. PPU creates an SPU thread group, passes the effective address of the struct as `arg0`
3. SPU does `mfc_get` (DMA read) to pull the struct into its local store
4. SPU performs SIMD computation, writes results into its local copy
5. SPU does `mfc_put` (DMA write) to push results back to main memory
6. PPU joins the thread group and reads the results
