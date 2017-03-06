/* tiny lz4 decompression library

define LZ4_IMPLEMENTATION exactly once in a .c file.

this library needs tinymalloc to be implmented somewhere
*/

#include <stdint.h>

void lz4_block_decompress (const uint8_t * restrict src, int src_len, uint8_t * restrict dst);
void lz4_stream_decompress (const uint8_t *src_data, int src_size, uint8_t *dst_data);


#ifdef TINYLZ4_IMPLEMENTATION // -----------------------------------------------------------------------------------------------------

#define MAGIC_LZ4 0x184D2204 
// ref. is see frame format https://cyan4973.github.io/lz4/lz4_Frame_format.md

// complete len already loaded with 0-15 first value
static unsigned lz4_extlen(unsigned len, const uint8_t * restrict *src)
{
    if (!len) 
        return 0;
    if (len==0xf) { // there is more
        uint8_t b;
        do {
            b=**src;
            (*src)++;
            len +=b;
        } while (b==0xff);
    }
    return len;
}

// lz4 block decoder
void lz4_block_decompress (const uint8_t * restrict src, int src_len, uint8_t * restrict dst)
{
    const uint8_t *end = src + src_len;
    do { 
        uint8_t tok = *src++; // read token

        // get literal len
        unsigned int len = lz4_extlen(tok>>4,&src);

        // copy literal data
        for (int i=len;i>0;--i)
            *dst++ = *src++;

        // match offset 
        uint8_t *match_ofs = dst - src[0] - (src[1]<<8); 
        src+=2;

        // get match len, adds 4 
        len=lz4_extlen(tok & 0xf,&src) + 4;

        // copy match data - use faster memcpy ?
        for (int i=len;i>0;--i)
            *dst++ = *match_ofs++;

    } while (src < end);
}

// LZ4 file format to RAM decoder
// note that lz4 encoded files MUST use --content-size encoding option. 

void lz4_stream_decompress (const uint8_t *src_data, int src_size, uint8_t *dst_data)
{
	// unsigned int decomp_size;

	if (*(uint32_t*)src_data != MAGIC_LZ4 ) {
		message("ERROR: Magic LZ4 header not found");
		die(1,4);
	}
	src_data += 4; // header = 0x184D2204 

	uint8_t flg = *src_data++;
	//message("flag is %x\n",flg);
	src_data++; 
	if ((flg & 1<<3) ) {
		// decomp_size = *(uint64_t *)src_data;
		src_data +=8; 
	}

	src_data +=1; // hc
	// uint32_t block_size = *(uint32_t*)src_data;
	src_data +=4;

	lz4_block_decompress(src_data,src_size,dst_data);

}
#endif // TINYLZ4_IMPLMENTATION