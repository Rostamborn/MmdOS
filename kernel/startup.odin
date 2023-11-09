#!/bin/bash

echo "Compiling C files..."
gcc -ffreestanding \
	-mno-red-zone \
	-mno-80387 \
	-mno-mmx \
	-mno-3dnow \
	-mno-sse \
	-mno-sse2 \
	-Wall \
	-Wextra \
    -o bin/kernel.o \
    -c kernel/kernel.c

ld -m elf_x86_64 \
    -nostdlib \
    -static \
    -pie \
    --no-dynamic-linker \
    -z text \
    -z max-page-size=0x1000 \
    -T linker.ld \
    -o bin/kernel.elf \
