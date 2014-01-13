#include "tinfl.c"
#define OUTBUF_SIZE 256
#include <stdio.h>

const char *src = "x\x9c\xcbH\xcd\xc9\xc9\xd7Q(\xcf/\xcaIQP\x04\x00&\xc7\x04\xca";
const int src_len = 22;
char dst[OUTBUF_SIZE];
#define MINIZ_NO_MALLOC

void main( void)
{
	int bytes = tinfl_decompress_mem_to_mem(
		(void *)&dst, OUTBUF_SIZE, 
		(const void*)src, src_len,
		TINFL_FLAG_PARSE_ZLIB_HEADER | 
		TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF | 
		TINFL_FLAG_COMPUTE_ADLER32
	  );
	//printf("msg: %s",dst);
	
}


