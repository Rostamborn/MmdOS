# MmdOS

### Overview

A 64-bit higher half kernel OS

## Installation

You need an [GCC compiler](https://gcc.gnu.org/), [ld linker](https://www.gnu.org/software/binutils/)
which are usually pre-installed on UNIX-like systems. Also [qemu](https://www.qemu.org/), [nasm](https://www.nasm.us/),
[xorriso](https://www.gnu.org/software/xorriso/) and [make](https://www.gnu.org/software/make/) must be available.

We also use [clang-format](https://releases.llvm.org/14.0.0/tools/clang/docs/ClangFormat.html) in the `make format` script, so make sure to have it installed in a development workspace.

In order to run the debugger, be sure to have [gdb](https://www.sourceware.org/gdb/).

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

if you want to use gdb:

```sh
make drun
```

then open another shell and move to the same directory and:

```sh
make gdb
```

#### Implemented features

| Feature           | State                 |
| ----------------- | --------------------- |
| Limine bootloader | works                 |
| kernel loading    | works                 |
| GDT               | works                 |
| IDT               | works                 |
| PIC               | works                 |
| PIT               | works                 |
| Keyboard          | works                 |
| printf            | works                 |
| basic prompt      | works                 |
| Memory Mapping    | works                 |
| process           | works                 |
| context switching | works                 |
| thread            | works                 |
| file system       | basic vfs implemented |
| USTar             | works                 |
| userspace         | needs more testing    |
| games             | snake and gameoflife work|
| programs          | cat, ls work          |

#### TODO for the forseeable future

| Feature to be implemented |
| ------- |
| maybe implement ACPI to gain access to more advanced devices |
| user space |
| support for simple file systems like FAT |
| support for ELF |
| syscalls to execute binaries like ELFs |
| simple hardware support like usb |

#### Description

The goal is to implement a fully functional OS which would have basic tools and
perhaps support libc and some Unix-like programs and applications.

#### Credits

As we are beginners in OSdev and low level programming, we have had to use and  
read extensive amount of sources to have some understanging on the subject matter.  
The names wouldn't fit here, but we shall reference [OSdev Projects](https://wiki.osdev.org/Projects) which contains a lot  
of very good projects that can be used as references for learning.
We definitely have a great appreciation for every single one of them that helped us in any ways.
