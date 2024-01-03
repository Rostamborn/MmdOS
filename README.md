# MmdOS

### Overview

A 64-bit higher half kernel OS

## Installation

You need an [GCC compiler](https://gcc.gnu.org/), [ld linker](https://www.gnu.org/software/binutils/)
which are usually pre-installed on UNIX-like systems. Also [qemu](https://www.qemu.org/), [nasm](https://www.nasm.us/),
[xorriso](https://www.gnu.org/software/xorriso/) and [make](https://www.gnu.org/software/make/) must be available.

We also use [clang-format](https://releases.llvm.org/14.0.0/tools/clang/docs/ClangFormat.html) in the `make format` script, so make sure to have it installed in a development workspace.

to compile and build in debug mode run:

```sh
make
```

to compile and build in production mode run:

```sh
make prod
```

if you want to run the os using qemu, execute:

```sh
make run
```

#### Implemented features

| Feature           | State |
| ----------------- | ----- |
| Limine bootloader | works |
| kernel loading    | works |
| GDT               | works |
| IDT               | works |
| PIC               | works |
| PIT               | works |
| Keyboard          | works |
| printf            | works |
| basic prompt      | works |
| Memory Mapping    | works |
| process           | half way done |
| contest switching | works |

#### TODO for the forseeable future

**_ In the future, rewrite the assignment to registers and etc. using Unions _**
| Feature to be implemented |
| ------- |
| Threads |
| better memory map |
| unique address spaces for processes |
| more features for process management |
| maybe implement ACPI to gain access to more advanced devices |
| user space |
| system calls |
| file system |

#### Description

The goal is to implement a fully functional OS which would have basic tools and 
perhaps support libc and some Unix-like programs and applications.

#### Credit

Special thanks to the contributers of [Lyre-OS](https://github.com/Lyre-OS/Lyre) 
for being one of the main references of ideas and implementation.
