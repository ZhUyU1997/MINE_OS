OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

SECTIONS { 
	. = 0x800000;
    .text :
	{
		PROVIDE(_text = .);
		arch/arm32/mach-x4418/head.o;
		*(.text*);
		PROVIDE(_etext = .);
	}
	
    . = ALIGN(4);
    .data :
	{
		PROVIDE(_data = .);
		*(.data);
		PROVIDE(_edata = .);
	}
	
	. = ALIGN(4);
    .rodata :
	{
		PROVIDE(_rodata = .);
		*(.rodata);
		PROVIDE(_erodata = .);
	}
    . = ALIGN(4);
	.bss :
	{
		PROVIDE(_bss = .);
		*(.bss);
		*(COMMON);
		PROVIDE(_ebss = .);
	}
	
	_end = .;
}
