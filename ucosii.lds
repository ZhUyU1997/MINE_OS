ENTRY(_start)
SECTIONS { 
	. = 0x30100000;
    .text :
	{
		_text = .;
		*(.text*);
		_etext = .;
	}
	
    . = ALIGN(4);
    .data :
	{
		_data = .;
		*(.data);
		_edata = .;
	}
	
	. = ALIGN(4);
    .rodata :
	{
		_rodata = .;
		*(.rodata);
		_erodata = .;
	}
    . = ALIGN(4);
	.bss :
	{
		__bss_start = .;
		_bss = .;
		*(.bss);
		*(COMMON);
		_ebss = .;
		__bss_end = .;
	}
	
	_end = .;
} 
