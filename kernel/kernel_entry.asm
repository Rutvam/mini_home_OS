[bits 64]
section .bss
	align 16
	global stack_bottom
	global stack_top

	stack_bottom:
		resb 16384 ; Fr: Reserver 16 Kio 
	stack_top:

section .text
	global _start
	global keyboard_handler_asm
	global outb
	extern main
	_start:
		cli ; Fr: Desactive les interuption
		cld ; Fr: Direction des chaine vers l'avant

		mov rsp, stack_top ; Fr: initialise la pile
		and rsp, -16 ; Fr: Aligne la pile sur 16 octets
		
		call main
		.hang:
			hlt
			jmp .hang

	keyboard_handler_asm:
		hlt
		jmp keyboard_handler_asm

	outb:
		; RDI = port
		; RSI = value
		mov dx, di
		mov al, sil
		out dx, al
		ret
