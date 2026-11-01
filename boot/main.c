#include <elf.h>
#include <efi.h>
#include <efilib.h>
#include <string.h>
#include "./function.h"

// Enter the kernel after UEFI Boot Services have been terminated.
static void JumpToKernel(Elf64_Ehdr *Header, struct LOADER_PARAMS *Params)
{
	typedef void (*KernelEntry)(struct LOADER_PARAMS *);

	KernelEntry Entry = (KernelEntry)(UINTN)Header->e_entry;

	Entry(Params);

	// A kernel normally never returns.
	while (1)
	{
		__asm__ volatile ("hlt");
	}
}

struct LOADER_PARAMS Params;

// Main UEFI application entry point.
EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_BOOT_SERVICES *BS = SystemTable->BootServices;

	EFI_STATUS status;

	EFI_LOADED_IMAGE *LoadedImage = NULL;

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem = NULL;
	EFI_FILE_PROTOCOL *Root = NULL;
	EFI_FILE_PROTOCOL *KernelFile = NULL;

	EFI_FILE_INFO *FileInfo = NULL;

	VOID *KernelBuffer = NULL;

	UINTN KernelSize;

	EFI_TIME now;
	EFI_GRAPHICS_OUTPUT_PROTOCOL *GOP;

	EFI_PHYSICAL_ADDRESS KernelAddress = 0;
	UINTN KernelPages = 0;

	EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;

	UINTN MemoryMapSize = 0;
	UINTN MapKey = 0;
	UINTN DescriptorSize = 0;
	UINT32 DescriptorVersion = 0;

	UINT8 MaxStepNumber = 11;
	UINT8 StepNumber = 0;

	InitializeLib(ImageHandle, SystemTable);



	// ---------------------------------------------------------------
	//  STEP 1: Loaded image
	// ---------------------------------------------------------------
	PrintStep(&StepNumber, MaxStepNumber, L"Getting loaded image information...");

	status = GetLoadedImage(ImageHandle, &LoadedImage);

	if (EFI_ERROR(status))
	{
		return status;
	}


	// ---------------------------------------------------------------
	//  STEP 2: Current time
	// ---------------------------------------------------------------
	PrintStep(&StepNumber, MaxStepNumber, L"Getting current time...");

	status = uefi_call_wrapper(RT->GetTime, 2, &now, NULL);

	if (EFI_ERROR(status))
	{
		return PrintError(L"GetTime", status);
	}

	Print(L"\t[OK] %02u/%02u/%04u %02u:%02u:%02u\r\n", now.Day, now.Month, now.Year, now.Hour, now.Minute, now.Second);

	// ---------------------------------------------------------------
	//  STEP 3: Getting FrameBuffer
	// ---------------------------------------------------------------
	PrintStep(&StepNumber, MaxStepNumber, L"Getting FrameBuffer");

	status = uefi_call_wrapper(BS->LocateProtocol, 3, &gEfiGraphicsOutputProtocolGuid, NULL, (void **)&GOP);

	if (EFI_ERROR(status))
	{
		return PrintError(L"Get FrameBuffer", status);
	}

	Print(L"\t[OK] ");

	// ---------------------------------------------------------------
	//  STEP 4: Filesystem
	// ---------------------------------------------------------------
	PrintStep(&StepNumber, MaxStepNumber, L"Opening filesystem...");

	status = OpenFilesystem(LoadedImage, &FileSystem);

	if (EFI_ERROR(status))
	{
		return status;
	}


	// ---------------------------------------------------------------
	//  STEP S: Root volume
	// ---------------------------------------------------------------
	PrintStep(&StepNumber, MaxStepNumber, L"Opening root volume...");

	status = OpenRoot(FileSystem, &Root);

	if (EFI_ERROR(status))
	{
		return status;
	}


	// ---------------------------------------------------------------
	//  STEP 6: Kernel file
	// ---------------------------------------------------------------
	PrintStep(&StepNumber, MaxStepNumber, L"Opening kernel.elf...");

	status = OpenKernel(Root, &KernelFile);

	if (EFI_ERROR(status))
	{
		return status;
	}


	/*
	 * ---------------------------------------------------------------
	 * STEP 6: Kernel information
	 * ---------------------------------------------------------------
	 */
	PrintStep(&StepNumber, MaxStepNumber, L"Reading kernel information...");

	status = GetKernelFileInfo(KernelFile, &FileInfo);

	if (EFI_ERROR(status))
	{
		return status;
	}

	KernelSize = FileInfo->FileSize;


	// ---------------------------------------------------------------
	//  STEP 7: Load kernel file into UEFI pool memory
	// ---------------------------------------------------------------
	PrintStep(&StepNumber, MaxStepNumber, L"Allocating kernel buffer...");

	status = LoadKernelFile(BS, KernelFile, KernelSize, &KernelBuffer);

	if (EFI_ERROR(status))
	{
		return status;
	}


	// ---------------------------------------------------------------
	//  STEP 8: ELF validation
	// ---------------------------------------------------------------
	PrintStep(&StepNumber, MaxStepNumber, L"Validating ELF kernel...");

	Elf64_Ehdr *Header = (Elf64_Ehdr *)KernelBuffer;

	status = ValidateELF(Header);

	if (EFI_ERROR(status))
	{
		uefi_call_wrapper(BS->FreePool, 1, KernelBuffer);
		return status;
	}

	PrintELFHeader(Header);


	// ---------------------------------------------------------------
	//  STEP 9: Load ELF segments
	// ---------------------------------------------------------------
	PrintStep(&StepNumber, MaxStepNumber, L"Loading ELF segments into kernel memory...");
	status = LoadELFSegments(BS, Header, KernelBuffer, &KernelAddress, &KernelPages);

	if (EFI_ERROR(status))
	{
		return status;
	}

	Print(L"\r\n[OK] Kernel segments loaded\r\n");


	// ---------------------------------------------------------------
	//  Prepare UEFI memory map
	// ---------------------------------------------------------------

	Print(L"\r\nPreparing UEFI memory map...\r\n");

	status = uefi_call_wrapper(BS->GetMemoryMap, 5, &MemoryMapSize, NULL, &MapKey, &DescriptorSize, &DescriptorVersion);

	if (status != EFI_BUFFER_TOO_SMALL)
	{
		return PrintError(L"Initial GetMemoryMap", status);
	}


	// Add some extra space because additional UEFI operations can
	// change the memory map.
	MemoryMapSize += 8 * DescriptorSize;

	Print(L"\tRequired memory map buffer: %lu bytes\r\n", MemoryMapSize);

	status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, MemoryMapSize, (VOID **)&MemoryMap);

	if (EFI_ERROR(status))
	{
		return PrintError(L"Memory map allocation", status);
	}

	Print(L"\t[OK] Memory map buffer allocated\r\n");


	// ---------------------------------------------------------------
	//  ExitBootServices()
	// ---------------------------------------------------------------
	while (1)
	{
		UINTN CurrentMapSize = MemoryMapSize;

		PrintStep(&StepNumber, MaxStepNumber, L"Attempting ExitBootServices()...");

		// This must be the last operation that changes the memory map before ExitBootServices().
		status = GetFinalMemoryMap(BS, MemoryMap, MemoryMapSize, &CurrentMapSize, &MapKey, &DescriptorSize, &DescriptorVersion);

		if (EFI_ERROR(status))
		{
			return PrintError(L"Final GetMemoryMap", status);
		}

		// Build the loader parameters BEFORE ExitBootServices().
		// No Boot Service may be called between GetMemoryMap() and ExitBootServices(), otherwise the MapKey can become invalid.
		Params.UEFI_Version = SystemTable->Hdr.Revision;

		Params.Bootloader_MajorVersion = 0;
		Params.Bootloader_MinorVersion = 1;

		Params.Memory_Map_Descriptor_Version = DescriptorVersion;
		Params.Memory_Map_Descriptor_Size = DescriptorSize;
		Params.Memory_Map = MemoryMap;
		Params.Memory_Map_Size = CurrentMapSize;

		Params.Kernel_BaseAddress = KernelAddress;
		Params.Kernel_Pages = KernelPages;

		Params.RTServices = SystemTable->RuntimeServices;
		Params.FileMeta = FileInfo;

		Params.ConfigTables = SystemTable->ConfigurationTable;
		Params.Number_of_ConfigTables = SystemTable->NumberOfTableEntries;

		Params.BootTime = now;

		Params.GPU_Configs->FrameBuffer = GOP->Mode->FrameBufferBase;
		Params.GPU_Configs->FrameBuffer_Size = GOP->Mode->FrameBufferSize;
		Params.GPU_Configs->width = GOP->Mode->Info->HorizontalResolution;
		Params.GPU_Configs->height = GOP->Mode->Info->VerticalResolution;
		Params.GPU_Configs->pitch = GOP->Mode->Info->PixelsPerScanLine;


		// This is the critical transition: After a successful ExitBootServices(), UEFI Boot Services are no longer available.
		status = uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, MapKey);


		if (status == EFI_SUCCESS)
		{
			break;
		}


		// The memory map changed between GetMemoryMap() and ExitBootServices().
		// We retrieve it again and retry.
		if (status == EFI_INVALID_PARAMETER)
		{
			Print(L"\t[WARNING] ExitBootServices returned EFI_INVALID_PARAMETER\r\n");
			continue;
		}


		return PrintError(L"ExitBootServices", status);
	}


	// ---------------------------------------------------------------
	//  UEFI IS NOW GONE
	// ---------------------------------------------------------------
	// From this point onward:
	// - no Print()
	// - no AllocatePool()
	// - no FreePool()
	// - no GetMemoryMap()
	// - no other UEFI Boot Services
	// 
	// The kernel is now responsible for the machine.


	// Jump to the ELF entry point.
	JumpToKernel(Header, &Params);
	return EFI_SUCCESS;
}
