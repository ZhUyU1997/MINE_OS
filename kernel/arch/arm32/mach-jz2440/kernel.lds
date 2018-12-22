OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

SECTIONS { 
	. = 0x30100000;
    .text :
	{
		PROVIDE(_text = .);
		arch/arm32/mach-jz2440/head.o;
		arch/arm32/mach-jz2440/init.o;
		arch/arm32/mach-jz2440/nand_ll.o;
		*(.text*);
		*(.init.text);
		*(.exit.text);
		PROVIDE(_etext = .);
	}

	.initcall ALIGN(8) :
	{
		PROVIDE(__initcall_start = .);
		KEEP(*(.initcall_0.text))
		KEEP(*(.initcall_1.text))
		KEEP(*(.initcall_2.text))
		KEEP(*(.initcall_3.text))
		KEEP(*(.initcall_4.text))
		KEEP(*(.initcall_5.text))
		KEEP(*(.initcall_6.text))
		KEEP(*(.initcall_7.text))
		KEEP(*(.initcall_8.text))
		KEEP(*(.initcall_9.text))
		PROVIDE(__initcall_end = .);
	}

	.exitcall ALIGN(8) :
	{
		PROVIDE(__exitcall_start = .);
		KEEP(*(.exitcall_9.text))
		KEEP(*(.exitcall_8.text))
		KEEP(*(.exitcall_7.text))
		KEEP(*(.exitcall_6.text))
		KEEP(*(.exitcall_5.text))
		KEEP(*(.exitcall_4.text))
		KEEP(*(.exitcall_3.text))
		KEEP(*(.exitcall_2.text))
		KEEP(*(.exitcall_1.text))
		KEEP(*(.exitcall_0.text))
		PROVIDE(__exitcall_end = .);
	}

    .data ALIGN(8) :
	{
		PROVIDE(_data = .);
		*(.data);
		PROVIDE(_edata = .);
	}
	
    .rodata ALIGN(8) :
	{
		PROVIDE(_rodata = .);
		*(.rodata);
		PROVIDE(_erodata = .);
	}

	.bss ALIGN(8) (NOLOAD) :
	{
		PROVIDE(_bss = .);
		*(.bss);
		*(COMMON);
		PROVIDE(_ebss = .);
	}
	
	_end = .;
} 
