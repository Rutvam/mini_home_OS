#include <elf.h>
#include <efi.h>
#include <efilib.h>
#include "./function.h"

static VOID *GlobalKernelBuffer = NULL;

// Print a small section header.
VOID PrintStep(UINT8 *step, UINT8 total, const CHAR16 *message)
{
	Print(L"\r\n[%lu/%lu] %s\r\n", *step, total, message);
	(*step)++;
}



// Print an EFI error and return the status.
EFI_STATUS PrintError(const CHAR16 *message, EFI_STATUS status)
{
	Print(L"\t[ERROR] %s: %r\r\n", message, status);
	return status;
}



// Allocate memory from the UEFI pool and print the resulting address range.
// AllocatePool does not give us a physical-address-style allocation request.
// UEFI chooses an available pool region for us.
EFI_STATUS AllocateKernelBuffer(EFI_BOOT_SERVICES *BS, UINTN Size, VOID **Buffer)
{
	EFI_STATUS status;

	status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, Size, Buffer);

	if (EFI_ERROR(status))
	{
		return PrintError(L"AllocatePool", status);
	}

	if (Size == 0 || Buffer == NULL)
	{
		return EFI_INVALID_PARAMETER;
	}

	Print(L"\t[OK] Pool allocation successful\r\n");
	Print(L"\t\tStart address : 0x%lx\r\n", (UINTN)*Buffer);
	Print(L"\t\tSize          : 0x%lx (%lu bytes)\r\n", Size, Size);
	Print(L"\t\tEnd address   : 0x%lx\r\n", (UINTN)*Buffer + Size - 1);

	return EFI_SUCCESS;
}


// Open the EFI loaded-image protocol.
EFI_STATUS GetLoadedImage(EFI_HANDLE ImageHandle, EFI_LOADED_IMAGE **LoadedImage)
{
	EFI_STATUS status;

	status = uefi_call_wrapper(BS->OpenProtocol, 6, ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)LoadedImage, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);

	if (EFI_ERROR(status))
	{
		return PrintError(L"LoadedImage protocol", status);
	}

	Print(L"\t[OK] LoadedImage protocol obtained\r\n");

	return EFI_SUCCESS;
}



// Open the filesystem containing the EFI application.
EFI_STATUS OpenFilesystem(EFI_LOADED_IMAGE *LoadedImage, EFI_SIMPLE_FILE_SYSTEM_PROTOCOL **FileSystem)
{
	EFI_STATUS status;

	status = uefi_call_wrapper(BS->HandleProtocol, 3, LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (VOID **)FileSystem);

	if (EFI_ERROR(status))
	{
		return PrintError(L"Simple File System protocol", status);
	}

	Print(L"\t[OK] Filesystem protocol obtained\r\n");

	return EFI_SUCCESS;
}



// Open the root directory of the EFI System Partition.
EFI_STATUS OpenRoot(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem, EFI_FILE_PROTOCOL **Root)
{
	EFI_STATUS status;

	status = uefi_call_wrapper(FileSystem->OpenVolume, 2, FileSystem, Root);

	if (EFI_ERROR(status))
	{
		return PrintError(L"Opening filesystem volume", status);
	}

	Print(L"\t[OK] Root volume opened\r\n");

	return EFI_SUCCESS;
}


// Open kernel.elf from the root directory.
EFI_STATUS OpenKernel(EFI_FILE_PROTOCOL *Root, EFI_FILE_PROTOCOL **KernelFile)
{
	EFI_STATUS status;

	status = uefi_call_wrapper(Root->Open, 5, Root, KernelFile, L"kernel.elf", EFI_FILE_MODE_READ, 0);

	if (EFI_ERROR(status))
	{
		return PrintError(L"Opening kernel.elf", status);
	}

	Print(L"\t[OK] kernel.elf opened\r\n");

	return EFI_SUCCESS;
}


// Retrieve the EFI_FILE_INFO structure of kernel.elf.
// GetInfo is normally called twice:
// 1. First call asks UEFI how much memory is needed.
// 2. Second call fills the allocated buffer.
EFI_STATUS GetKernelFileInfo(EFI_FILE_PROTOCOL *KernelFile, EFI_FILE_INFO **FileInfo)
{
	EFI_STATUS status;
	UINTN FileInfoSize = 0;

	status = uefi_call_wrapper(KernelFile->GetInfo, 4, KernelFile, &gEfiFileInfoGuid, &FileInfoSize, NULL);

	if (status != EFI_BUFFER_TOO_SMALL)
	{
		return PrintError(L"GetInfo size query", status);
	}

	status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, FileInfoSize, (VOID **)FileInfo);

	if (EFI_ERROR(status))
	{
		return PrintError(L"Allocating EFI_FILE_INFO", status);
	}

	status = uefi_call_wrapper(KernelFile->GetInfo, 4, KernelFile, &gEfiFileInfoGuid, &FileInfoSize, *FileInfo);

	if (EFI_ERROR(status))
	{
		uefi_call_wrapper(BS->FreePool, 1, *FileInfo);
		*FileInfo = NULL;

		return PrintError(L"Reading EFI_FILE_INFO", status);
	}

	Print(L"\t[OK] File information retrieved\r\n");
	Print(L"\t\tFile size: %lu bytes\r\n", (*FileInfo)->FileSize);

	return EFI_SUCCESS;
}



// Read the entire ELF file into a UEFI pool allocation.
EFI_STATUS LoadKernelFile(EFI_BOOT_SERVICES *BS, EFI_FILE_PROTOCOL *KernelFile, UINTN KernelSize, VOID **KernelBuffer)
{
	EFI_STATUS status;
	UINTN ReadSize = KernelSize;

	status = AllocateKernelBuffer(BS, KernelSize, KernelBuffer);

	if (EFI_ERROR(status))
	{
		return status;
	}

	status = uefi_call_wrapper(KernelFile->Read, 3, KernelFile, &ReadSize, *KernelBuffer);

	if (EFI_ERROR(status))
	{
		uefi_call_wrapper(BS->FreePool, 1, *KernelBuffer);
		*KernelBuffer = NULL;

		return PrintError(L"Reading kernel.elf", status);
	}

	if (ReadSize != KernelSize)
	{
		Print(L"\t[ERROR] Incomplete read: %lu/%lu bytes\r\n", ReadSize, KernelSize);

		uefi_call_wrapper(BS->FreePool, 1, *KernelBuffer);
		*KernelBuffer = NULL;

		return EFI_LOAD_ERROR;
	}

	Print(L"\t[OK] %lu bytes loaded into memory\r\n", ReadSize);

	return EFI_SUCCESS;
}


// Validate the ELF header.
// At this stage we only check the properties needed by this bootloader:
//  - ELF magic
//  - 64-bit ELF
//  - x86-64 architecture
EFI_STATUS ValidateELF(Elf64_Ehdr *Header)
{
	if (Header->e_ident[EI_MAG0] != ELFMAG0 || Header->e_ident[EI_MAG1] != ELFMAG1 || Header->e_ident[EI_MAG2] != ELFMAG2 || Header->e_ident[EI_MAG3] != ELFMAG3)
	{
		Print(L"\t[ERROR] Invalid ELF magic\r\n");
		return EFI_LOAD_ERROR;
	}

	if (Header->e_ident[EI_CLASS] != ELFCLASS64)
	{
		Print(L"\t[ERROR] ELF is not 64-bit\r\n");
		return EFI_LOAD_ERROR;
	}

	if (Header->e_machine != EM_X86_64)
	{
		Print(L"\t[ERROR] ELF architecture is not x86-64\r\n");
		return EFI_LOAD_ERROR;
	}

	Print(L"\t[OK] Valid ELF64 x86-64 file\r\n");
	return EFI_SUCCESS;
}

EFI_STATUS ValidateEntryPoint(Elf64_Ehdr *Header, UINTN KernelSize)
{
	if (Header->e_type != ET_EXEC)
	{
		Print(L"\t[ERROR] Kernel is not ET_EXEC\r\n");
		return EFI_LOAD_ERROR;
	}

	if (Header->e_phentsize != sizeof(Elf64_Phdr) || Header->e_phnum == 0)
	{
		Print(L"\t[ERROR] Invalid program header table\r\n");
		return EFI_LOAD_ERROR;
	}

	// Check that the program header table is inside the ELF file.
	if (Header->e_phoff > KernelSize || Header->e_phnum > (KernelSize - Header->e_phoff) / sizeof(Elf64_Phdr))
	{
		Print(L"\t[ERROR] Program header table out of bounds\r\n");
		return EFI_LOAD_ERROR;
	}

	Elf64_Phdr *ProgramHeaders = (Elf64_Phdr *)((UINT8 *)Header + Header->e_phoff);

	UINT64 Entry = Header->e_entry;
	BOOLEAN EntryFound = FALSE;
	BOOLEAN LoadSegmentFound = FALSE;

	for (UINT16 i = 0; i < Header->e_phnum; i++)
	{
		Elf64_Phdr *Segment = &ProgramHeaders[i];
		if (Segment->p_type != PT_LOAD)
		{
			continue;
		}

		LoadSegmentFound = TRUE;
		if (Segment->p_filesz > Segment->p_memsz)
		{
			return EFI_LOAD_ERROR;
		}

		if (Segment->p_offset > KernelSize || Segment->p_filesz > KernelSize - Segment->p_offset)
		{
			Print(L"\t[ERROR] Segment exceeds ELF file bounds\r\n");
			return EFI_LOAD_ERROR;
		}

		if (Segment->p_memsz > UINT64_MAX - Segment->p_vaddr)
		{
			Print(L"\t[ERROR] Segment address overflow\r\n");
			return EFI_LOAD_ERROR;
		}

		// Entry must point inside file-backed executable code.
		if ((Segment->p_flags & PF_X) && Entry >= Segment->p_vaddr && Entry - Segment->p_vaddr < Segment->p_filesz)
		{
			EntryFound = TRUE;
		}
	}

	if (!LoadSegmentFound || !EntryFound)
	{
		Print(L"\t[ERROR] Entry point is not inside executable code\r\n");
		return EFI_LOAD_ERROR;
	}

	Print(L"\t[OK] Entry point belongs to an executable PT_LOAD segment\r\n");
	return EFI_SUCCESS;
}

// Print the important ELF header information.
void PrintELFHeader(Elf64_Ehdr *Header)
{
	Print(L"\r\n\t--- ELF HEADER ---\r\n");
	Print(L"\tEntry       : 0x%lx\r\n", Header->e_entry);
	Print(L"\tPH offset   : 0x%lx\r\n", Header->e_phoff);
	Print(L"\tPH size     : %u\r\n", Header->e_phentsize);
	Print(L"\tPH count    : %u\r\n", Header->e_phnum);
}


// Verify that the memory copy was successful.
// This function does not modify anything.
UINTN VerifyCopy(UINT8 *Destination, UINT8 *Source, UINTN Size)
{
	UINTN errors = 0;
	for (UINTN i = 0; i < Size; i++)
	{
		if (Destination[i] != Source[i])
		{
			errors++;
			if (errors <= 20)
			{
				Print(L"\t\tDifference at 0x%lx: RAM=0x%02x ELF=0x%02x\r\n", i, Destination[i], Source[i]);
			}
		}
	}
	return errors;
}



// Copy one ELF PT_LOAD segment into its requested memory address.
// 
// The first copy uses CopyMem().
// 
// If verification fails, a manual byte-by-byte copy is attempted.
// This is intentionally useful for debugging CopyMem() versus memory
// allocation/addressing problems.
EFI_STATUS LoadSegment(EFI_BOOT_SERVICES *BS, Elf64_Phdr *ph)
{
	EFI_STATUS status;

	UINTN Pages;
	EFI_PHYSICAL_ADDRESS KernelAddress;

	UINT8 *Destination;
	UINT8 *Source;

	UINTN errors;

	// Calculate the number of 4 KiB pages required.
	Pages = (ph->p_memsz + 0xFFF) / 0x1000;

	KernelAddress = ph->p_vaddr;

	Print(L"\r\n\t--- PT_LOAD ---\r\n");
	Print(L"\tOffset       : 0x%lx\r\n", ph->p_offset);
	Print(L"\tVirtual addr : 0x%lx\r\n", ph->p_vaddr);
	Print(L"\tFile size    : 0x%lx (%lu bytes)\r\n", ph->p_filesz, ph->p_filesz);
	Print(L"\tMemory size  : 0x%lx (%lu bytes)\r\n", ph->p_memsz, ph->p_memsz);
	Print(L"\tPages        : %lu\r\n", Pages);

	Print(L"\tRequested address: 0x%lx\r\n", KernelAddress);

	// Allocate the physical pages at the exact address requested by
	// the ELF program header.

	status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAddress, EfiLoaderData, Pages, &KernelAddress);

	if (EFI_ERROR(status))
	{
		PrintError(L"AllocatePages", status);
		return status;
	}

	if (KernelAddress != ph->p_vaddr)
	{
		Print(L"\t[WARNING] Address changed!\r\n");
		Print(L"\t\tRequested : 0x%lx\r\n", ph->p_vaddr);
		Print(L"\t\tObtained  : 0x%lx\r\n", KernelAddress);
	}
	else
	{
		Print(L"\t[OK] Requested address successfully allocated\r\n");
		Print(L"\t\tAddress : 0x%lx\r\n", KernelAddress);
	}

	// Calculate source and destination addresses.
	Destination = (UINT8 *)(UINTN)ph->p_vaddr;


	// Source is inside KernelBuffer.
	// NOTE: KernelBuffer itself is allocated by AllocatePool and contains the complete kernel.elf file.
	Source = (UINT8 *)GlobalKernelBuffer + ph->p_offset;

	Print(L"\tSource address      : 0x%lx\r\n", (UINTN)Source);
	Print(L"\tDestination address : 0x%lx\r\n", (UINTN)Destination);
	Print(L"\tCopy size           : 0x%lx (%lu bytes)\r\n", ph->p_filesz, ph->p_filesz);


	// Check the first bytes before copying.
	Print(L"\tFirst source bytes  : ");

	for (UINTN i = 0; i < 8 && i < ph->p_filesz; i++)
	{
		Print(L"%02x ", Source[i]);
	}

	Print(L"\r\n");

	// Test whether the destination memory is actually writable.
	// WARNING:
	// - This modifies the first four bytes temporarily.
	// - They are restored immediately afterwards by CopyMem().
	Print(L"\tTesting destination memory...\r\n");

	UINT8 Test0 = Destination[0];
	UINT8 Test1 = Destination[1];
	UINT8 Test2 = Destination[2];
	UINT8 Test3 = Destination[3];

	Destination[0] = 0xAA;
	Destination[1] = 0xBB;
	Destination[2] = 0xCC;
	Destination[3] = 0xDD;

	Print(L"\t\tRead back: %02x %02x %02x %02x\r\n", Destination[0], Destination[1], Destination[2], Destination[3]);
	
	// Restore the original bytes.
	Destination[0] = Test0;
	Destination[1] = Test1;
	Destination[2] = Test2;
	Destination[3] = Test3;

	// Perform the normal copy.
	Print(L"\tCopying segment with CopyMem()...\r\n");

	// Fr: Desactiver car il ne copy rien, il faut faire un copy manuel
	// CopyMem(Destination, Source, ph->p_filesz);

	// errors = VerifyCopy(Destination, Source, ph->p_filesz);
	errors = 1; // Fr: Provoquer une erreur 
	if (errors != 0) {
		Print(L"\t[WARNING] CopyMem() verification failed: %lu errors\r\n", errors);

		// Differential test: If CopyMem() is somehow responsible, a normal C loop should behave differently.
		Print(L"\tTrying manual byte-by-byte copy...\r\n");

		for (UINTN i = 0; i < ph->p_filesz; i++)
		{
			Destination[i] = Source[i];
		}

		errors = VerifyCopy(Destination, Source, ph->p_filesz);

		if (errors != 0)
		{
			Print(L"\t[ERROR] Manual copy also failed: %lu errors\r\n", errors);
			return EFI_LOAD_ERROR;
		}
		Print(L"\t[OK] Manual copy succeeded\r\n");
	} else {
		Print(L"\t[OK] CopyMem() verification succeeded\r\n");
	}

	// The part between p_filesz and p_memsz is not stored in the ELF
	// file. This normally contains .bss and must therefore be zeroed.
	if (ph->p_memsz > ph->p_filesz)
	{
		UINTN BssSize = ph->p_memsz - ph->p_filesz;
		Print(L"\tZeroing BSS: %lu bytes\r\n", BssSize);
		SetMem(Destination + ph->p_filesz, BssSize, 0);
		Print(L"\t[OK] BSS initialized\r\n");
	}

	return EFI_SUCCESS;
}


// Load every PT_LOAD segment from the ELF file.
EFI_STATUS LoadELFSegments(EFI_BOOT_SERVICES *BS, Elf64_Ehdr *Header, VOID *KernelBuffer, EFI_PHYSICAL_ADDRESS *KernelBaseAddress, UINTN *KernelPages)
{
	Elf64_Phdr *ProgramHeaders;

	ProgramHeaders = (Elf64_Phdr *)( (UINT8 *)KernelBuffer + Header->e_phoff );

	for (UINTN i = 0; i < Header->e_phnum; i++)
	{
		Elf64_Phdr *ph = &ProgramHeaders[i];

		if (ph->p_type != PT_LOAD)
		{
			continue;
		}


		// GlobalKernelBuffer is used by LoadSegment() because the segment source address must point inside the complete ELF file.
		GlobalKernelBuffer = KernelBuffer;

		EFI_STATUS status = LoadSegment(BS, ph);

		if (EFI_ERROR(status))
		{
			return status;
		}

		// For the current kernel there is only one PT_LOAD segment.
		// 
		// If multiple PT_LOAD segments are introduced later, this
		// should be changed so that the complete kernel range is stored
		// rather than simply keeping the last segment.
		*KernelBaseAddress = ph->p_vaddr;
		*KernelPages = (ph->p_memsz + 0xFFF) / 0x1000;
	}

	return EFI_SUCCESS;
}



// Retrieve the final UEFI memory map before ExitBootServices().
// The buffer must already exist before this function is called.
EFI_STATUS GetFinalMemoryMap(EFI_BOOT_SERVICES *BS, EFI_MEMORY_DESCRIPTOR *MemoryMap, UINTN MemoryMapSize, UINTN *CurrentMapSize, UINTN *MapKey, UINTN *DescriptorSize, UINT32 *DescriptorVersion)
{
	return uefi_call_wrapper(BS->GetMemoryMap, 5, CurrentMapSize, MemoryMap, MapKey, DescriptorSize, DescriptorVersion);
}




