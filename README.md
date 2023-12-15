# MmdOS

### Overview
A 64-bit higher half kernel OS

## Installation
You need an [GCC compiler](https://gcc.gnu.org/), [ld linker](https://www.gnu.org/software/binutils/) 
which are usually pre-installed on UNIX-like systems. Also [qemu](https://www.qemu.org/) and [nasm](https://www.nasm.us/) 
[xorriso](https://www.gnu.org/software/xorriso/) must be available. 
Then run:
```sh
make
make run
```
which will build, run and clean.

#### Implemented features
| Feature | State |
| ------- | -------|
| Limine bootloader | works |
| kernel loading | works |
| GDT | works |
| IDT | works |
| PIC | works |
| PIT | works |
| Keyboard | works |
| printf | works |

#### TODO for the forseeable future
*** In the future, rewrite the assignment to registers and etc. using Unions ***
| Feature to be implemented |
| ------- |
| Video Output |
| Memory Management |
| TSS and context switching |
| Definition for CPUs and saving state |

#### Description
The `C` files and potential `asm` files are compiled into object files in the `bin` 
directory and then will be linked by the linker `ld` and the `linker.ld` script with 
appropriate flags and the linked result will be put in `disk` directory. here the `iso` 
will be created and `limine` will be installed on it and the result of the process would 
be a bootable `iso`.
