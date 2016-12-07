/* This linker script is almost as a normal bitbox script, but linking to sram or ccmram */

ENTRY(InterruptVectors)

MEMORY
{
	flash (rx)		: ORIGIN = 0x08000000, LENGTH = 1024K
	ccmram (rwx)	: ORIGIN = 0x10000000, LENGTH = 64K
	sram (rwx)		: ORIGIN = 0x20000000, LENGTH = 112K
    sram2 (rwx)     : ORIGIN = 0x2001C000, LENGTH = 16K
}

__isr_vector_sram = 0x2001C000-0x200;

SECTIONS
{
    .text :
    {
        . = ALIGN(4);
        _text = .;
        __isr_vector_start = .;
        KEEP(*(.isr_vector))
        __isr_vector_end = .;
        . = ALIGN(4);
        *(.text*)
        *(.rodata*)
        _etext = .;
        _sidata = .;
    } > sram

    .data : AT(_sidata)
    {
        . = ALIGN(4);
        _sdata = .;
        *(vtable)
        . = ALIGN(4);
        *(.data*)
        _edata = .;
    } > sram

    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss*)
        . = ALIGN(4);
        *(COMMON)
        _ebss = .;
    } > sram

    
    .ccm (NOLOAD) :
    {
      _sccm = .;
      *(.ccm)
      _eccm = .;
    } > ccmram
}
