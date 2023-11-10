# MamadOS

### Overview
A 64-bit higher half kernel OS

## Installation
You need an GCC compiler, [qemu](https://www.qemu.org/) and [nasm](https://www.nasm.us/) installed. 
Then run:
```sh
./boom
```

#### Implemented features
| Feature | State |
| ------- | -------|
| Limine bootloader | works |
| kernel loading | works |
| printing to stdout for debugging | works |

#### TODO for the forseeable future
*** create a functiong `Makefile` to automate the make phase(currently using bash which is not ideal)***
| Feature |
| ------- |
| make it higher halved |
| IDT |
| Timer |
| Keyboard |

#### Description
The `C` files and potential `asm` files are compiled into object files in the `bin` 
directory and then will be linked by the linker `ld` and the `linker.ld` script with 
appropriate flags and the linked result will be put in `disk` directory. here the `iso` 
will be created and `limine` will be installed on it and the result of the process would 
be a bootable `iso`.
