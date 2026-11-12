#include <efi.h>
#include <efilib.h>
#include <stdint.h>
#include "./idt/IDT.h"
#include "./gdt/gdt.h"
#include "./serial/serial.h"
#include "./function/function.h"
#include "./data/global_value.h"
#include "./pong/main_pong.h"
#include "./shell/main_shell.h"
#include "./function/graphics/graphics.h"
#include "./shutdown/main_shutdown.h"
#include "../keyboard_driver/main.h"

int kernel_main(struct LOADER_PARAMS *params)
{
	// 1. Initialize serial port COM1 (UART 16550) for immediate debug logging
	serial_init();

	serial_print("\r\n=======================================================\r\n");
	serial_print(" [BOOT] mini_home_os kernel initializing...\r\n");
	serial_print("=======================================================\r\n");
	serial_print("[BOOT] UART 16550 serial driver initialized on COM1 (0x3F8, 115200 8N1)\r\n");
	serial_print("[BOOT] Kernel initialized successfully\r\n");

	// Verify bootloader parameters
	if (params == 0)
	{
		serial_print("[FAIL] LOADER_PARAMS pointer is NULL!\r\n");
		while (1)
		{
			__asm__ volatile ("cli; hlt");
		}
	}

	var.params = params; // save parameters pointer

	// Output early boot debug messages: resolution, framebuffer address, pitch, size
	serial_printf("[BOOT] LOADER_PARAMS pointer:  0x%016lx\r\n", (uint64_t)(uintptr_t)params);
	serial_printf("[BOOT] GOP Framebuffer base:  0x%016lx\r\n", params->GPU_Configs.FrameBuffer);
	serial_printf("[BOOT] GOP Framebuffer size:  %lu bytes\r\n", params->GPU_Configs.FrameBuffer_Size);
	serial_printf("[BOOT] GOP Resolution:        %ux%u\r\n", params->GPU_Configs.width, params->GPU_Configs.height);
	serial_printf("[BOOT] GOP PixelsPerScanline: %u\r\n", params->GPU_Configs.pitch);

	// 2. Kernel Handover Verification (Visual Proof of Life):
	// Paint the entire screen with a solid color (dark slate blue: 0x001E3F66)
	// by writing directly to framebuffer memory.
	if (params->GPU_Configs.FrameBuffer != 0)
	{
		volatile uint32_t *fb = (volatile uint32_t *)(uintptr_t)params->GPU_Configs.FrameBuffer;
		uint32_t width = params->GPU_Configs.width;
		uint32_t height = params->GPU_Configs.height;
		uint32_t pitch = params->GPU_Configs.pitch;
		uint32_t dark_slate = 0x001E3F66; // Dark slate blue

		for (uint32_t y = 0; y < height; y++)
		{
			for (uint32_t x = 0; x < width; x++)
			{
				fb[y * pitch + x] = dark_slate;
			}
		}
		serial_printf("[BOOT] Framebuffer filled with solid color (0x%08x) - Visual Proof of Life OK\r\n", dark_slate);
	}
	else
	{
		serial_print("[BOOT] WARN: Framebuffer address is 0, skipping visual fill\r\n");
	}

	// 3. CPU Architecture Setup
	// A. Set up 64-bit Global Descriptor Table (GDT) and load it using lgdt
	serial_print("[BOOT] Setting up 64-bit GDT...\r\n");
	gdt_init();
	serial_print("[BOOT] 64-bit GDT loaded successfully (CS=0x08, DS/SS=0x10)\r\n");

	// B. Create Interrupt Descriptor Table (IDT) with ISR handlers for CPU exceptions
	// (especially #GP 13 and #PF 14 so crashes produce diagnostics rather than silent reboots)
	serial_print("[BOOT] Setting up 64-bit IDT and CPU exception ISRs...\r\n");
	init_idt();
	serial_print("[BOOT] 64-bit IDT loaded with 32 exception handlers (#GP 13, #PF 14 active)\r\n");

	// 4. Initialize kernel subsystems
	my.init.function = init_function;
	my.init.function(1);
	my.init.var();
	serial_print("[BOOT] Kernel subsystems and variables initialized\r\n");

	serial_print("=======================================================\r\n");
	serial_print("[BOOT] Kernel initialization sequence complete.\r\n");
	serial_print("=======================================================\r\n");

	while (1)
	{
		__asm__ volatile ("hlt");
	}

	return 0;
}

// Ensure compatibility with any caller expecting 'main'
int main(struct LOADER_PARAMS *params)
{
	return kernel_main(params);
}
