ISO_FILENAME = MmdOS.iso
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
	-Wextra \
    -mcmodel=large \
	-fno-stack-protector \

override LDFLAGS += \
	-m elf_x86_64 \
    -nostdlib \
    -static \
    -pie \
    --no-dynamic-linker \
    -z text \
    -z max-page-size=0x1000 \
    -T linker.ld \

override NASMFLAGS += \
	-felf64 \

override XORRISOFLAGS += \
	-as mkisofs \
	-b limine/limine-cd.bin \
	-no-emul-boot \
	-boot-load-size 4 \
	-boot-info-table \
	--efi-boot \
	limine/limine-cd-efi.bin \
	-efi-boot-part \
	--efi-boot-image \
	--protective-msdos-label disk \

LIMINE_DEPLOY := ./disk/limine/limine-deploy

SRC_DIRECTORY := ./
KERNEL_DIR := ./src/kernel
LIB_DIR := ./src/lib
OBJECTS_DIR := ./obj
# C_SRCS = $(wildcard *.c)
# ASM_SRCS = $(wildcard *.asm)
KERNEL_C_FILES :=  $(shell find $(KERNEL_DIR) -name "*.c")
KERNEL_HEADER_FILES :=  $(shell find $(KERNEL_DIR) -name "*.h")
KERNEL_ASSEMBLY_FILES := $(shell find $(KERNEL_DIR) -name "*.asm")

LIB_C_FILES :=  $(wildcard $(LIB_DIR)/*.c)
LIB_ASSEMBLY_FILES := $(wildcard $(LIB_DIR)/*.asm)

TARGET := disk/kernel.elf

OBJS := $(pathsubst $(KERNEL_DIR)/%.c)
OBJS += $(pathsubst $(KERNEL_DIR)/%.asm)

all: $(OBJS)
	@echo "making kernel"
	make kernel/development -B

	@echo "making iso"
	make iso -B

	@echo "deploying lemine"
	make deploy-limine -B

prod: $(OBJS)
	@echo "making kernel"
	make kernel/production -B

	@echo "making iso"
	make iso -B

	@echo "deploying lemine"
	make deploy-limine -B

kernel/development: $(KERNEL_C_FILES, LIB_C_FILES, KERNEL_ASSEMBLY_FILES)
	@echo "compiling c files to objects"
	$(DEFAULT_CC) $(DEFAULT_CFLAGS) -I $(SRC_DIRECTORY) -D PROD_MODE=0 -c $(KERNEL_C_FILES) ${LIB_C_FILES}
	mv *.o $(OBJECTS_DIR)

	@echo "compiling assembly files to objects"
	nasm ${KERNEL_ASSEMBLY_FILES} ${NASMFLAGS} -o $(OBJECTS_DIR)/interrupt_vector.o

	@echo "linking..."
	$(DEFAULT_LD) $(LDFLAGS) -o $(TARGET) \
	$(OBJECTS_DIR)/*.o

	@echo "created kernel"

kernel/production: $(KERNEL_C_FILES, LIB_C_FILES, KERNEL_ASSEMBLY_FILES)
	@echo "compiling c files to objects"
	$(DEFAULT_CC) $(DEFAULT_CFLAGS) -I $(SRC_DIRECTORY) -D PROD_MODE=1 -c $(KERNEL_C_FILES) ${LIB_C_FILES}
	mv *.o $(OBJECTS_DIR)

	@echo "compiling assembly files to objects"
	nasm ${KERNEL_ASSEMBLY_FILES} ${NASMFLAGS} -o $(OBJECTS_DIR)/interrupt_vector.o

	@echo "linking..."
	$(DEFAULT_LD) $(LDFLAGS) -o $(TARGET) \
	$(OBJECTS_DIR)/*.o

	@echo "created kernel"

iso: 
	@echo "building iso"
	xorriso $(XORRISOFLAGS) \
	-o $(ISO_FILENAME)
	@echo "iso built"

deploy-limine:
	sudo $(LIMINE_DEPLOY) $(ISO_FILENAME)

build/%.c.o: %.c
	@echo "Compiling C files..."
	@mkdir -p $(@D)
	$(DEFAULT_CC) $(DEFAULT_CFLAGS) -c $< 

build/%.asm.o: %.asm
	@echo "Assembling file: $<"
	@mkdir -p $(@D)
	nasm $(NASMFLAGS) $< -c -o $@

run: $(ISO_FILENAME)
	qemu-system-x86_64 -serial stdio $(ISO_FILENAME)

.PHONY: clean

clean:
	rm $(OBJECTS_DIR)/* $(ISO_FILENAME) $(TARGET)

format: $(KERNEL_C_FILES)
	clang-format -i $(KERNEL_C_FILES) $(KERNEL_HEADER_FILES)

drun: $(ISO_FILENAME)
	qemu-system-x86_64 -s -S -serial stdio $(ISO_FILENAME)

gdb:
	gdb -x gdb_commands	
