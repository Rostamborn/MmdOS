ISO_FILENAME := build/mamadOS.iso
override DEFAULT_CC := gcc
override DEFAULT_LD := ld
override DEFAULT_CFLAGS += \
	-ffreestanding \
	-mno-red-zone \
	-mno-80387 \
	-mno-mmx \
	-mno-3dnow \
	-mno-sse \
	-mno-sse2 \
	-Wall \
	-Wextra

override LDFLAGS += \
	-nostdlib \
    -static \
    -pie \
    --no-dynamic-linker \
	-T linker.ld

override NASMFLAGS += \
	-f elf64 \
	-Wall \

C_SRCS := $(wildcard *.c)
ASM_SRCS := $(wildcard *.asm)
TARGET := build/kernel.elf

OBJS = $(pathsubst %.c, build/%.c.o, $(C_SRCS))
OBJS += $(pathsubst %.asm, build/%.asm.o, $(ASM_SRCS))

all: $(OBJS)
	@echo "Linking..."
	ld -o $(TARGET) $(LDFLAGS) $(OBJS)
	@echo "Program Linked, placed at $(TARGET)"

build/%.c.o: %.c
	@echo "Compiling C files..."
	@mkdir -p $(@D)
	$(DEFAULT_CC) $(DEFAULT_CFLAGS) -c $< -o $@

build/%.asm.o: %.asm
	@echo "Assembling file: $<"
	@mkdir -p $(@D)
	nasm $(NASMFLAGS) $< -c -o $@

run: build/mamadOS.iso
	qemu-system-x86_64 -cdrom -monitor $(ISO_FILENAME)

clean :
	rm -r build/*.o bulid/*.bin build/*.elf build/*.iso
