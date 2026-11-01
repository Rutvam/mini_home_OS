ENTRY(_start)

SECTIONS
{
	. = 0x100000;

	.text : ALIGN(16)  
	{  
		*(.text._start)  
		*(.text)  
	}  

	.rodata : ALIGN(16)  
	{  
		*(.rodata)  
	}  

	.data : ALIGN(16)  
	{  
		*(.data)  
	}  

	.bss : ALIGN(16)
	{
		*(COMMON)
		*(.bss .bss.*)
	}

	/DISCARD/ :  
	{  
		*(.note.gnu.property)  
		*(.comment)  
		*(.eh_frame)  
	}
}
