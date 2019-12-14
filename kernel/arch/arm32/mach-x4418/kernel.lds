OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

STACK_UND_SIZE = 0x12;
STACK_ABT_SIZE = 0x12;
STACK_IRQ_SIZE = 0x12;
STACK_FIQ_SIZE = 0x12;
STACK_SRV_SIZE = 0x100000;

MEMORY
{
	ram  : org = 0x40100000, len = 128M - 1M
	dma  : org = 0x48000000, len = 128M
	heap : org = 0x50000000, len = 256M
}

SECTIONS { 
    .text :
	{
		PROVIDE(__image_start = .);
		PROVIDE(__text_start = .);
		arch/arm32/mach-x4418/head.o;
		*(.text*);
		*(.init.text);
		*(.exit.text);
		*(.glue*);
		PROVIDE(__text_end = .);
	} > ram

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
	} > ram

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
	} > ram

	.class.type ALIGN(8) :
	{
		PROVIDE(__class_type_start = .);
		KEEP(*(.class.type))
		PROVIDE(__class_type_end = .);
	} > ram

	.class.table ALIGN(8) :
	{
		PROVIDE(__class_table_start = .);
		KEEP(*(.class.table))
		PROVIDE(__class_table_end = .);
	} > ram

    .data ALIGN(8) :
	{
		PROVIDE(__data_start = .);
		*(.data*)
		PROVIDE(__data_end = .);
	} > ram
	
	.dtree ALIGN(8) :
	{
		PROVIDE(__dtree_start = .);
		KEEP(*(.dtree));
		PROVIDE(__dtree_end = .);
	} > ram

    .rodata ALIGN(8) :
	{
		PROVIDE(__rodata_start = .);
		*(.rodata*);
		PROVIDE(__rodata_end = .);
		PROVIDE(__image_end = .);
	} > ram

	.bss ALIGN(8) (NOLOAD) :
	{
		PROVIDE(__bss_start = .);
		*(.bss*);
		*(COMMON);
		PROVIDE(__bss_end = .);
	} > ram
	
	.stack ALIGN(8) (NOLOAD) :
	{
		PROVIDE(__stack_start = .);
		PROVIDE(__stack_und_start = .);
		. += STACK_UND_SIZE;
		PROVIDE(__stack_und_end = .);
		. = ALIGN(8);
		PROVIDE(__stack_abt_start = .);
		. += STACK_ABT_SIZE;
		PROVIDE(__stack_abt_end = .);
		. = ALIGN(8);
		PROVIDE(__stack_irq_start = .);
		. += STACK_IRQ_SIZE;
		PROVIDE(__stack_irq_end = .);
		. = ALIGN(8);
		PROVIDE(__stack_fiq_start = .);
		. += STACK_FIQ_SIZE;
		PROVIDE(__stack_fiq_end = .);
		. = ALIGN(8);
		PROVIDE(__stack_srv_start = .);
		. += STACK_SRV_SIZE;
		PROVIDE(__stack_srv_end = .);
		. = ALIGN(8);
		PROVIDE(__stack_end = .);
	} > ram

	PROVIDE(__end = ORIGIN(dma));

	.dma ALIGN(8) (NOLOAD) :
	{
		PROVIDE(__dma_start = ORIGIN(dma));
		PROVIDE(__dma_end = ORIGIN(dma) + LENGTH(dma));
	} > dma

	.heap ALIGN(8) (NOLOAD) :
	{
		PROVIDE(__heap_start = ORIGIN(heap));
		PROVIDE(__heap_end = ORIGIN(heap) + LENGTH(heap));
	} > heap
} 
