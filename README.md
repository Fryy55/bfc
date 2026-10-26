# Brainfuck Compiler
My attempt at making a Brainfuck compiler from first principles

See [Cross-Compilation](#cross-compilation) below for all available system configuration

# Cross-Compilation
To make a cross-compiling compiler (e.g. compile Brainfuck code for arm64 macOS on a Linux amd64 machine) pass `-DBFC_TARGET_SYSTEM="<ISA>;<ABI>;<OS>"` to your CMake configuration command. Below is a table of all supported systems and the values to pass

| OS    | ABI    | ISA   | Support    |
| ----- | ------ | ----- | ---------- |
| Linux | sysV   | amd64 | 🕐 Planned |
| macOS | arm64e | arm64 | 🕐 Planned |

> [!IMPORTANT]
> All supported architectures must be strictly _little endian_, using _two's complement_

This way, in order to build a cross-compiler to amd64 Linux, you would pass `-DBFC_TARGET_SYSTEM="amd64;sysV;Linux"`

Not passing anything will use current system's configuration, if available

> [!TIP]
> You might need to add some missing platform-specific headers to your environment in case you're building a cross-compiler

# License
This project is distributed under the **Apache-2.0 License**.

See `LICENSE` for permissions, conditions and limitations.