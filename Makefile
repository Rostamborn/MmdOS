C_SOURCES = $(wildcard kernel/*.c drivers/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h)

OBJ = ${C_SOURCES:.c=.o}

all: run

run: os-image
	qemu-system-x86_64 os-image

os-image: boot/boot_sect.bin kernel.bin
	cat $^ > os-image

# boot/boot_sect.bin: boot/boot_sect.asm
# 	nasm $< -f bin -o $@

kernel.bin: kernel/kernel_entry.o ${OBJ}
	ld -o $@ -Ttext 0x1000 $^ --oformat binary

# kernel/kernel_entry.o: kernel/kernel_entry.asm
# 	nasm $< -f elf64 -I '/' -o $@

%.o : %.c ${HEADERS}
	gcc -ffreestanding -c $< -o $@

# os-image: boot/boot_sect.bin kernel.bin
# 	cat $^ > os-image
#
# kernel.bin: boot/kernel_entry.o ${OBJ}
# 	ld -o $@ -Ttext 0x1000 $^ --oformat binary
#
# %.o : %.c ${HEADERS}
# 	gcc -ffreestanding -c $< -o $@
#
%.o : %.asm
	nasm $< -f elf64 -o $@
#
%.bin : %.asm
	nasm $< -f bin -o $@
# 	nasm $< -f bin -I 'boot/' -o $@

clean:
	rm -fr *.bin *.dis *.o os-image
	rm -fr kernel/*.o boot/*.bin drivers/*.o
#
# kernel.o: kernel.c
# 	gcc -ffreestanding -c $< -o $@
#
# kernel_entry.o: kernel_entry.asm
# 	nasm $< -f elf64 -o $@
#
# boot_sect.bin: boot_sect.asm
# 	nasm $< -f bin -I ’../../16 bit /’ -o $@
#
# clean:
# 	rm -fr *. bin*. dis*.o os-image *.map *.o
#
# kernel.dis : kernel.bin
# 	ndisasm -b 32 $< > $@
