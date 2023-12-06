# MamadOS

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
| printing to stdout for debugging | works |
| IDT | works |
| PIC | works |
| PIT | works |

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

#### IDT 
Refer to [IDT](https://wiki.osdev.org/Interrupt_Descriptor_Table) to know what an IDT is and checkout 
[IDT tutorial](https://wiki.osdev.org/Interrupts_tutorial) to see how they must be implemented. 
To populate the IDT with ISRs, we have a concept of interrupt stubs. The goal is to enter interrupt handlers 
gracefully and return from them gracefully. Using the power of assembly macros, we create stubs(which are basically functions). 
We populate the `IDT` with these stubs using `set_interrupt_descriptor`. If we call any of them, an error code(could be a dummy error code) 
and the number associated with the stub gets pushed onto the stack. Then it is redirected to `interrupt_stub` which pushes general purpose 
registers in order to save the state and calls `interrupt_dispatch` which takes the stack pointer as parameter. In `interrupt_dispatch` we 
can now properly handle the raised interrupt with their associated number. How do we access this number? Using the stack pointer we just pushed. 
To ease the access to the contents of the stack, we create a struct which mirrors the stack in terms of it's contents and also order(`interrupt_frame` structure).
there's still a lot more to finish handling interrupts...
