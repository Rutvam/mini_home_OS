# Homemade Mini OS

## Description

This project is a homemade mini operating system designed to eventually support games and provide a basic shell.

## How to install the project

```shell
git clone https://www.github.com/Rutvam/mini_home_os.git
```

```shell
cd /home/username/mini_home_os
```

```shell
make clean && make compile-all
```

## Tools

- gcc
- nasm
- make
- QEMU or VBox (VirtualBox)

  Required for contributors who want to test the project.

- sudo

**Don't forget to install any missing tools.**

### How to install the basic compilation tools

**Arch Linux:**
```shell
sudo pacman -S gcc nasm make
```

**Debian / Ubuntu:**
```shell
sudo apt install gcc nasm make
```

## File Structure

```shell
❯ tree ./mini_home_os
./mini_home_os
├── boot
│   ├── function.c
│   ├── function.h
│   └── main.c
├── compile
│   ├── boot
│   │   ├── boot.efi
│   │   ├── boot.so
│   │   ├── function.o
│   │   └── main.o
│   ├── kernel
│   │   └── kernel.elf
│   └── keyboard_driver
│       ├── main_keyboard_driver.o
│       └── main.o
├── doc
│   ├── ACPI.md
│   └── base_rust.md
├── esp.img
├── esp.vdi
├── kernel
│   ├── core
│   ├── data
│   │   ├── global_value.c
│   │   ├── global_value.h
│   │   └── graphics_value
│   │       ├── police.c
│   │       └── police.h
│   ├── function
│   │   ├── function.c
│   │   ├── function.h
│   │   ├── graphics
│   │   │   ├── graphics.c
│   │   │   └── graphics.h
│   │   ├── math
│   │   │   ├── math.c
│   │   │   └── math.h
│   │   ├── memory
│   │   │   ├── memory.c
│   │   │   └── memory.h
│   │   ├── print_function
│   │   │   ├── print_function.c
│   │   │   └── print_function.h
│   │   └── random
│   │       ├── random.c
│   │       └── random.h
│   ├── idt
│   │   ├── IDT.c
│   │   └── IDT.h
│   ├── kernel_entry.asm
│   ├── linker.ld
│   ├── main.c
│   ├── pong
│   │   ├── main_pong.c
│   │   └── main_pong.h
│   ├── shell
│   │   ├── main_shell.c
│   │   └── main_shell.h
│   └── shutdown
│       ├── main_shutdown.c
│       └── main_shutdown.h
├── keyboard_driver
│   ├── main.c
│   └── main.h
├── LICENSE
├── Makefile
├── ovmf_vars.fd
└── README.md

22 directories, 47 files
```

## Future Improvements

- [X] Replace the BIOS-compatible bootloader (`./boot/boot.asm`) with a UEFI-compatible bootloader (`./boot/boot.c`).

  - [X] Organize the bootloader code and ensure that GCC reports no compilation errors.

  - [X] Update the `Makefile` to support the new bootloader structure.

  - [X] Perform one final test to verify that the new bootloader works, then remove the old bootloader (`./boot/boot.asm`).

    If both `make QEMU` and `make VBox` run without any issues, the following command can be used to complete this step:

    ```shell
    rm -rf ./boot/boot.asm
    ```

- [X] Finalize kernel compatibility with the new bootloader.

  - [X] Receive bootloader parameters in the kernel.

  - [X] Verify the kernel entry point.

  - [X] Organize the kernel structure.

  - [X] Update the `Makefile` to support the new kernel structure.

- [ ] Fix bugs.

- [ ] Start implementing the shutdown function.

## Author

- Rutvam

  - <https://www.github.com/Rutvam>

## Third-Party Code and Acknowledgements

### KNNSpeed
    
- Original project: [Simple-UEFI-Bootloader](https://github.com/KNNSpeed/Simple-UEFI-Bootloader)

- Original author: KNNSpeed

- Original version: V2.3
    
The following code was reused from the original project:

- `struct GPU_CONFIG`

- `struct LOADER_PARAMS`

- Related loader parameter definitions

The original code is distributed under its Custom Attribution License ("PD with Credit").

The original source and its licensing terms are acknowledged and respected.

**Date obtained:** 
