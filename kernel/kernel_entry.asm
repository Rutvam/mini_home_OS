[bits 64]
section .bss
	align 16
	global stack_bottom
	global stack_top

	stack_bottom:
		resb 16384 ; 16 KiB kernel stack
	stack_top:

section .text
	global _start
	global inb
	global outb
	global gdt_load
	global idt_load
	global keyboard_handler_asm

	extern kernel_main
	extern exception_handler
	extern keyboard_handler_c

	_start:
		cli
		cld

		; Handover parameter check:
		; Under System V ABI, parameter is in RDI.
		; Under Microsoft x64 ABI (UEFI), parameter is in RCX.
		; If RDI is NULL but RCX is not NULL, copy RCX to RDI.
		test rdi, rdi
		jnz .param_ready
		mov rdi, rcx
	.param_ready:

		; Set up 16-byte aligned kernel stack
		mov rsp, stack_top
		and rsp, -16
		
		; Call kernel_main with RDI pointing to LOADER_PARAMS
		call kernel_main

	.hang:
		cli
		hlt
		jmp .hang

	; I/O Port operations
	inb:
		; RDI = port
		mov dx, di
		xor eax, eax
		in al, dx
		ret

	outb:
		; RDI = port
		; RSI = value
		mov dx, di
		mov al, sil
		out dx, al
		ret

	; 64-bit GDT loader
	gdt_load:
		; RDI = pointer to struct GDTR
		lgdt [rdi]

		; Reload data segment selectors (0x10 = Kernel Data Segment)
		mov ax, 0x10
		mov ds, ax
		mov es, ax
		mov fs, ax
		mov gs, ax
		mov ss, ax

		; Reload CS to 0x08 (Kernel Code Segment) using 64-bit far return
		push qword 0x08
		lea rax, [rel .reload_cs]
		push rax
		retfq
	.reload_cs:
		ret

	; 64-bit IDT loader
	idt_load:
		; RDI = pointer to struct IDTR
		lidt [rdi]
		ret

	; Common Exception ISR Stub
	isr_common_stub:
		; Push general-purpose registers
		push rax
		push rbx
		push rcx
		push rdx
		push rsi
		push rdi
		push rbp
		push r8
		push r9
		push r10
		push r11
		push r12
		push r13
		push r14
		push r15

		; Pass pointer to interrupt_frame in RDI (1st argument in SysV ABI)
		mov rdi, rsp
		cld
		call exception_handler

		; Restore general-purpose registers
		pop r15
		pop r14
		pop r13
		pop r12
		pop r11
		pop r10
		pop r9
		pop r8
		pop rbp
		pop rdi
		pop rsi
		pop rdx
		pop rcx
		pop rbx
		pop rax

		; Remove vector and error code
		add rsp, 16

		iretq

	; Exception ISR definition macros
	%macro ISR_NOERRCODE 1
	global isr%1
	isr%1:
		push qword 0        ; Dummy error code
		push qword %1       ; Exception vector
		jmp isr_common_stub
	%endmacro

	%macro ISR_ERRCODE 1
	global isr%1
	isr%1:
		push qword %1       ; Exception vector
		jmp isr_common_stub
	%endmacro

	ISR_NOERRCODE 0   ; #DE Divide-by-Zero
	ISR_NOERRCODE 1   ; #DB Debug
	ISR_NOERRCODE 2   ; NMI Non-Maskable Interrupt
	ISR_NOERRCODE 3   ; #BP Breakpoint
	ISR_NOERRCODE 4   ; #OF Overflow
	ISR_NOERRCODE 5   ; #BR Bound Range Exceeded
	ISR_NOERRCODE 6   ; #UD Invalid Opcode
	ISR_NOERRCODE 7   ; #NM Device Not Available
	ISR_ERRCODE   8   ; #DF Double Fault
	ISR_NOERRCODE 9   ; Coprocessor Segment Overrun
	ISR_ERRCODE   10  ; #TS Invalid TSS
	ISR_ERRCODE   11  ; #NP Segment Not Present
	ISR_ERRCODE   12  ; #SS Stack-Segment Fault
	ISR_ERRCODE   13  ; #GP General Protection Fault
	ISR_ERRCODE   14  ; #PF Page Fault
	ISR_NOERRCODE 15  ; Reserved
	ISR_NOERRCODE 16  ; #MF x87 FPU Floating-Point Error
	ISR_ERRCODE   17  ; #AC Alignment Check
	ISR_NOERRCODE 18  ; #MC Machine Check
	ISR_NOERRCODE 19  ; #XM SIMD Floating-Point Exception
	ISR_NOERRCODE 20  ; #VE Virtualization Exception
	ISR_ERRCODE   21  ; #CP Control Protection Exception
	ISR_NOERRCODE 22  ; Reserved
	ISR_NOERRCODE 23  ; Reserved
	ISR_NOERRCODE 24  ; Reserved
	ISR_NOERRCODE 25  ; Reserved
	ISR_NOERRCODE 26  ; Reserved
	ISR_NOERRCODE 27  ; Reserved
	ISR_NOERRCODE 28  ; Reserved
	ISR_NOERRCODE 29  ; Reserved
	ISR_NOERRCODE 30  ; Reserved
	ISR_NOERRCODE 31  ; Reserved

	; Keyboard IRQ handler stub
	keyboard_handler_asm:
		push rax
		push rbx
		push rcx
		push rdx
		push rsi
		push rdi
		push rbp
		push r8
		push r9
		push r10
		push r11
		push r12
		push r13
		push r14
		push r15

		cld
		call keyboard_handler_c

		pop r15
		pop r14
		pop r13
		pop r12
		pop r11
		pop r10
		pop r9
		pop r8
		pop rbp
		pop rdi
		pop rsi
		pop rdx
		pop rcx
		pop rbx
		pop rax

		iretq
