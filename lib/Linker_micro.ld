/* This is the standard bitbox micro linker script */

ENTRY(InterruptVectors)

MEMORY
{
	flash (rx)		: ORIGIN = 0x08000000, LENGTH = 512K /* no bootloader */
	sram (rwx)		: ORIGIN = 0x20000000, LENGTH = 96K
}
stack_size = 1024;


/* define start / end of stack at end of sram */
_sstack = ORIGIN(sram)+LENGTH(sram); 
_estack = _sstack - stack_size; 
 
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
    } > sram

    /* fake ccm to sram */
    .ccm (NOLOAD) :
    {
      _sccm = .;
      *(.ccm)
      _eccm = .;

    } > sram
}
