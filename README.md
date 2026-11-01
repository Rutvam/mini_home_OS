# Mini OS maison

## description

Ce projet est un mini OS maison permetant de jouer a des jeux dans le future et avoir un mini shell.

## Structure des fichier

```zsh
❯ tree
.
├── boot
│   ├── function.c
│   ├── function.h
│   ├── main.c
│   └── main.rs
├── compile
│   ├── boot
│   │   ├── boot.efi
│   │   ├── function.o
│   │   └── main.o
│   └── kernel
│       ├── function.o
│       ├── global_value.o
│       ├── graphics.o
│       ├── IDT.o
│       ├── kernel.elf
│       ├── kernel_entry.o
│       ├── main.o
│       ├── math.o
│       ├── memory.o
│       ├── police.o
│       ├── pong
│       │   └── main_pong.o
│       ├── print_function.o
│       ├── random.o
│       ├── shell
│       │   └── main_shell.o
│       └── shutdown
│           └── main_shutdown.o
├── doc
│   └── ACPI.md
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
│   ├── main_keyboard_driver.c
│   └── main_keyboard_driver.h
├── LICENSE
├── Makefile
├── ovmf_vars.fd
└── README.md

24 directories, 58 files
```

## Futur amelioration

- [X] Preparer la fonction shutdown dans le dossier `./shutdown` avec le fichier principal `./shutdown/main_shutdown.c`.

- [X] Passer de bootloader compatible avec le BIOS (`./boot/boot.asm`) vers bootloader compatible avec l'UEFI (`./boot/boot.c`).

	- [X] Structurer le bootloader et faire en sorte que le compilateur gcc ne trouve aucune faute.

	- [X] Modifier le `Makefile` pour le rendre compatible à la nouvelle structure du bootloader.

	- [X] Esseiler une derrnier fois si le bootloader final fonctionne, pour finalment supprimer l'ancien bootloader (`./boot/boot.asm`).

		Si `make QEMU` et `make VBox` fonctionne sans problème on utilise cette commande pour conslure cette etape.

		```shell
		rm -rf ./boot/boot.asm
		```

- [ ] Finaliser la compatibilité du kernel avec le nouveau bootloader.

	- [X] Recevoir les paramètres du bootloader dans le kernel.

	- [X] Vérifier l'entrée du kernel.

	- [ ] Ranger la structure du kernel.

	- [ ] Modifier le `Makefile` pour le rendre compatible à la nouvelle structure du kernel.

	- [ ] Exploiter les informations GOP dans le kernel.

- [ ] Commencer la fonction shutdown.

## Auteur

- Rutvam

	- <https://www.github.com/Rutvam>

## Auteur indirect

- KNNSpreed

	- <https://github.com/KNNSpeed/Simple-UEFI-Bootloader>

	- Rutvam a copier:

		```./boot/function.h
		// ...
		struct GPU_CONFIG {
			UINT64 FrameBuffer;
			UINT64 FrameBuffer_Size;
			UINT32 width;
			UINT32 height;
			UINT32 pitch;
		};
		
		struct LOADER_PARAMS {
			UINT32					UEFI_Version;
			UINT32					Bootloader_MajorVersion;
			UINT32					Bootloader_MinorVersion;
		
			UINT32					Memory_Map_Descriptor_Version;
			UINTN					Memory_Map_Descriptor_Size;
			EFI_MEMORY_DESCRIPTOR	*Memory_Map;
			UINTN					Memory_Map_Size;
		
			EFI_PHYSICAL_ADDRESS	Kernel_BaseAddress;
			UINTN					Kernel_Pages;
		
			EFI_RUNTIME_SERVICES	*RTServices;
			struct GPU_CONFIG		*GPU_Configs;
			EFI_FILE_INFO			*FileMeta;
			EFI_CONFIGURATION_TABLE	*ConfigTables;
			UINTN					Number_of_ConfigTables;
		
			EFI_TIME				BootTime;
		};
		
		extern struct LOADER_PARAMS Params;
		// ...
		```
