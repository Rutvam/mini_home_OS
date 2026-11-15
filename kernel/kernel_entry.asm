[bits 64]
section .bss
	align 16
	global stack_bottom
	global stack_top

	stack_bottom:
		resb 16384 ; Fr: Reserver 16 Kio 
	stack_top:

section .text
	extern main

	global _start
	_start:
		cli ; Fr: Desactive les interuption
		cld ; Fr: Direction des chaine vers l'avant

		mov rsp, stack_top ; Fr: initialise la pile
		and rsp, -16 ; Fr: Aligne la pile sur 16 octets
		
		call main
		.hang:
			hlt
			iret
			jmp .hang

	global keyboard_handler_asm
	extern keyboard_handler_c
	keyboard_handler_asm:
		push rax
		push rcx
		push rdx
		push rsi
		push rdi
		push r8
		push r9
		push r10
		push r11

		call keyboard_handler_c

		pop r11
		pop r10
		pop r9
		pop r8
		pop rdi
		pop rsi
		pop rdx
		pop rcx
		pop rax
		iretq

	global isr_gp_handler_asm
	extern isr_gp_handler_c
	isr_gp_handler_asm:
		push rax
		push rcx
		push rdx
		push rsi
		push rdi
		push r8
		push r9
		push r10
		push r11
		; Le CPU a empilé: RIP, CS, RFLAGS, RSP, SS, ET un code d'erreur d'exception au sommet.
		call isr_gp_handler_c
		pop r11
		pop r10
		pop r9
		pop r8
		pop rdi
		pop rsi
		pop rdx
		pop rcx
		pop rax
		add rsp, 8
		iretq

	global gdt_load
	gdt_load:
		; RDI contient l'adresse de la structure GDTR passée en argument (ABI System V x86_64)
		lgdt [rdi]
		; Recharger le segment de code avec un far jump vers le sélecteur 0x08
		push 0x08
		lea rax, [rel .reload_CS]
		push rax
		retfq

	.reload_CS:
		; Recharger les segments de données avec le sélecteur 0x10
		mov ax, 0x10
		mov ds, ax
		mov es, ax
		mov fs, ax
		mov gs, ax
		mov ss, ax
		ret

	global outb
	outb:
		; RDI = port
		; RSI = value
		mov dx, di
		mov al, sil
		out dx, al
		ret

	global inb
	inb:
		mov dx, di
		xor eax, eax
		in al, dx
		ret
