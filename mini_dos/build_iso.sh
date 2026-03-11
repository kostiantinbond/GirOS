#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"
ISO_DIR="$BUILD_DIR/isodir"
OUTPUT_ISO="$ROOT_DIR/GirOS.iso"

require_tool() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "Missing required tool: $1" >&2
        exit 1
    fi
}

require_tool nasm
require_tool gcc
require_tool ld
require_tool grub-mkrescue

mkdir -p "$BUILD_DIR" "$ISO_DIR/boot/grub"

nasm -f elf32 "$ROOT_DIR/src/boot.asm" -o "$BUILD_DIR/boot.o"
gcc -m32 -ffreestanding -fno-pic -fno-stack-protector -nostdlib -c "$ROOT_DIR/src/kernel.c" -o "$BUILD_DIR/kernel.o"
ld -m elf_i386 -T "$ROOT_DIR/linker.ld" -o "$BUILD_DIR/kernel.bin" "$BUILD_DIR/boot.o" "$BUILD_DIR/kernel.o"

cp "$BUILD_DIR/kernel.bin" "$ISO_DIR/boot/kernel.bin"
cp "$ROOT_DIR/boot/grub/grub.cfg" "$ISO_DIR/boot/grub/grub.cfg"

grub-mkrescue -o "$OUTPUT_ISO" "$ISO_DIR" >/dev/null 2>&1

echo "ISO created: $OUTPUT_ISO"
