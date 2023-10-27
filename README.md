# MamadOS

### Overview
A simple 32-bit (maybe 64-bit in the future) OS

## Installation
You need an [Odin](http://odin-lang.org/) compiler, [qemu](https://www.qemu.org/) and [nasm](https://www.nasm.us/) installed. 
Then run:
```sh
./boom
```

#### Implemented features
| Feature | State |
| ------- | -------|
| Limine bootloader | working state |
| kernel loading | working state |

#### TODO for the forseeable future
*** FIX THE LIMINE RELATED STUFF DURING STARTUP. LIKE THE Term_Request AND Frame_Buffers ****
| Feature |
| ------- |
| implement IDT(interrupt descriptor table) |
| implement paging |
| driver for screen output |

#### Description
After using the appropriate `Limine` binaries, the `boot/limine.cfg` is checked to see where is the kernel entry point. 
We have a `startup()` procedure which is the entry point whilst linking using the `linker` file in `kernel` directory. 
It must be mentioned that we've ported some `Limine` functionalities according to the protocol in the `kernel/limine/limine.odin` which 
Limine will look for symbols and magic numbers to figure them out. 
We must do stuff according to the boot protocol and have our environment set (like memory mapping, frame buffers, etc.) 
At the end of `startup()` we call the kernel by `kmain()`.
