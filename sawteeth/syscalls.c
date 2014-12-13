#include <unistd.h>

#ifndef EMULATOR
caddr_t _sbrk(int incr) {
	extern char _ebss; // Defined by the linker
	static char *heap_end;
	char *prev_heap_end;

	if (heap_end == 0) {
		heap_end = &_ebss;
	}
	prev_heap_end = heap_end;

	if (heap_end + incr >  (char*)0x20020000)
	{
		for(;;); //OOM!
	}

	heap_end += incr;
	return (caddr_t) prev_heap_end;
}

void _fstat() {}
void _open() {}
void _write() {}
void _close() {}
void _isatty() {}
void _lseek() {}
void _read() {}
#endif
