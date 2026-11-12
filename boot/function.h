#include <elf.h>
#include <efi.h>
#include <efilib.h>
#ifndef FUNCTION_BOOT_H
#define FUNCTION_BOOT_H

// These structures come from:
// https://github.com/KNNSpeed/Simple-UEFI-Bootloader
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
	struct GPU_CONFIG		GPU_Configs;
	EFI_FILE_INFO			*FileMeta;
	EFI_CONFIGURATION_TABLE	*ConfigTables;
	UINTN					Number_of_ConfigTables;

	EFI_TIME				BootTime;
};

extern struct LOADER_PARAMS Params;

VOID PrintStep(UINT8 *step, UINT8 total, const CHAR16 *message);
EFI_STATUS PrintError(const CHAR16 *message, EFI_STATUS status);
EFI_STATUS AllocateKernelBuffer(EFI_BOOT_SERVICES *BS, UINTN Size, VOID **Buffer);
EFI_STATUS OpenFilesystem(EFI_LOADED_IMAGE *LoadedImage, EFI_SIMPLE_FILE_SYSTEM_PROTOCOL **FileSystem);
EFI_STATUS GetLoadedImage(EFI_HANDLE ImageHandle, EFI_LOADED_IMAGE **LoadedImage);
EFI_STATUS OpenRoot(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem, EFI_FILE_PROTOCOL **Root);
EFI_STATUS OpenKernel(EFI_FILE_PROTOCOL *Root, EFI_FILE_PROTOCOL **KernelFile);
EFI_STATUS GetKernelFileInfo(EFI_FILE_PROTOCOL *KernelFile, EFI_FILE_INFO **FileInfo);
EFI_STATUS LoadKernelFile(EFI_BOOT_SERVICES *BS, EFI_FILE_PROTOCOL *KernelFile, UINTN KernelSize, VOID **KernelBuffer);
EFI_STATUS ValidateELF(Elf64_Ehdr *Header);
void PrintELFHeader(Elf64_Ehdr *Header);
UINTN VerifyCopy(UINT8 *Destination, UINT8 *Source, UINTN Size);
EFI_STATUS LoadSegment(EFI_BOOT_SERVICES *BS, Elf64_Phdr *ph);
EFI_STATUS LoadELFSegments(EFI_BOOT_SERVICES *BS, Elf64_Ehdr *Header, VOID *KernelBuffer, EFI_PHYSICAL_ADDRESS *KernelBaseAddress, UINTN *KernelPages);
EFI_STATUS GetFinalMemoryMap(EFI_BOOT_SERVICES *BS, EFI_MEMORY_DESCRIPTOR *MemoryMap, UINTN MemoryMapSize, UINTN *CurrentMapSize, UINTN *MapKey, UINTN *DescriptorSize, UINT32 *DescriptorVersion);

#endif
