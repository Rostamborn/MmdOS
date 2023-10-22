all : run

run : os-image
	qemu-system-x86_64 os-image

os-image: boot_sect.bin kernel.bin
	cat $^ > os-image

kernel.bin: kernel.o
	ld -o $@ -Ttext 0x1000 $^ --oformat binary

kernel.o : kernel_entry.o
	odin build kernel.odin -file -build-mode:obj -out:kernel.o -target:freestanding_amd64_sysv

# Same as the above rule .
kernel_entry.o : kernel_entry.asm
	nasm $< -f elf64 -o $@

boot_sect.bin : boot_sect.asm
	nasm $< -f bin -o $@

# Clear away all generated files .
clean :
	rm -fr *.bin *.dis *.o os-image *.map

kernel.dis : kernel.bin
	ndisasm -b 32 $< > $@
