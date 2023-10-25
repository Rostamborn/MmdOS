# MamadOS

### Overview
A simple 32-bit (maybe 64-bit in the future) OS

## Installation
You need an [Odin](http://odin-lang.org/) compiler, [qemu](https://www.qemu.org/) and [nasm](https://www.nasm.us/) installed. 
Then run:
```sh
./build
```

#### Implemented features
| Feature | State |
| ------- | -------|
| simple boot loader | working state |
| kernel loading | working state |

#### TODO for the forseeable future
| Feature |
| ------- |
| implement IDT(interrupt descriptor table) |
| implement paging |
| driver for screen output |

#### Description
We get the boot loader at an offset, setup our stack, then load the kernel. 
But in order to tell the CPU which function is our main function(in freestanding platforms, this is not obvious), 
we must provide a kernel entry point which we do in `kernel_entry.asm`. 
We build the Kernel as an object file and then link it with other needed object files create in the 
`bin` directory. 
We define our low level functions(which will be used extensively in the kernel and drivers) in `cpu` package 
and use Odin's `foreign` system to import and use those assembly functions as Odin does not support(as of yet) 
inline assembly. Anytime we need those functions, we just import the `cpu` package and use the procedures defined in 
`cpu.odin`.
