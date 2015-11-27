/* This is the standard bitbox linker script

Note that origin of flash is 0x8004000 to keep room for the bootloader.

*/

ENTRY(InterruptVectors)

MEMORY
{
	flash (rx)		: ORIGIN = 0x08004000, LENGTH = 1008K
	ccmram (rwx)	: ORIGIN = 0x10000000, LENGTH = 64K
	sram (rwx)		: ORIGIN = 0x20000000, LENGTH = 128K
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
    } > flash

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
		end = . ;
    } > sram

    
    .ccm (NOLOAD) :
    {
      _sccm = .;
      *(.ccm)
      _eccm = .;
    } > ccmram

    /* strange bugs even if C only is used ... */
    /DISCARD/ :
    {
        *(.ARM.exidx)
    }
}
