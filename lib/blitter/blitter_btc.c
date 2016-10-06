#include "blitter.h"

// --- BTC4 (single and 2x magnification)
// ---------------------------------------------------------------------------

void btc4_line (object *o);

object * btc4_new(const uint32_t *btc, int16_t x, int16_t y, int16_t z)
{
    object *o = blitter_new();    
    if (!o) return 0; // error

    // generic attributes
    o->x=x; o->ry=y;o->z=z;

    o->w=*btc++;
    o->h=*btc++;

    // palette start + blocks
    o->data = (uint32_t*)btc;
    
    o->line=btc4_line;
    return o;
}

// switch16 version (fastest so far. could be made faster by coding to ASM (?) or blitting 4 lines at a time - full blocks, loading palette progressively ...)
void btc4_line (object *o)  
{
    int line=vga_line-o->ry;
    uint16_t *palette = (uint16_t*)(o->data);

    // palette is 256 u16 so 128 u32 after start (no padding)
    // data advances width/4 words per block (and a block is line/4)
    uint32_t *src =  ((uint32_t*)(o->data)) + 128 + (o->w/4)*(line / 4); 
    //uint32_t linemask = 3<<((line&3)*4); // test of bits starts with this mask, 16,20,24,28

    int x = (o->x) & 0xfffffffe; // ensure word aligned ... case unaligned TBD
    uint32_t *dst = (uint32_t*) (&draw_buffer[x]); 
    int n=o->w/4;

    // __builtin_expect(((n > 0) && ((n&7)==0));
    for (int i=0;i<n;i++)
    {
        uint32_t word_block = *src++; 

        uint16_t c1=palette[ word_block >>24]; 
        uint16_t c2=palette[(word_block >>16) & 0xff]; 

        switch(word_block>>((line&3)*4) & 0xf)
         {
            case   0 : *dst++ = (c2<<16)|c2; *dst++ = (c2<<16)|c2; break;
            case   1 : *dst++ = (c2<<16)|c1; *dst++ = (c2<<16)|c2; break;
            case   2 : *dst++ = (c1<<16)|c2; *dst++ = (c2<<16)|c2; break;
            case   3 : *dst++ = (c1<<16)|c1; *dst++ = (c2<<16)|c2; break;
            case   4 : *dst++ = (c2<<16)|c2; *dst++ = (c2<<16)|c1; break;
            case   5 : *dst++ = (c2<<16)|c1; *dst++ = (c2<<16)|c1; break;
            case   6 : *dst++ = (c1<<16)|c2; *dst++ = (c2<<16)|c1; break;
            case   7 : *dst++ = (c1<<16)|c1; *dst++ = (c2<<16)|c1; break;
            case   8 : *dst++ = (c2<<16)|c2; *dst++ = (c1<<16)|c2; break;
            case   9 : *dst++ = (c2<<16)|c1; *dst++ = (c1<<16)|c2; break;
            case  10 : *dst++ = (c1<<16)|c2; *dst++ = (c1<<16)|c2; break;
            case  11 : *dst++ = (c1<<16)|c1; *dst++ = (c1<<16)|c2; break;
            case  12 : *dst++ = (c2<<16)|c2; *dst++ = (c1<<16)|c1; break;
            case  13 : *dst++ = (c2<<16)|c1; *dst++ = (c1<<16)|c1; break;
            case  14 : *dst++ = (c1<<16)|c2; *dst++ = (c1<<16)|c1; break;
            case  15 : *dst++ = (c1<<16)|c1; *dst++ = (c1<<16)|c1; break;
        }
    }
} //__attribute__ ((hot))

// btc4_2x for pixel_doubled data

void btc4_2x_line (object *o);

object * btc4_2x_new(const uint32_t *btc, int16_t x, int16_t y, int16_t z)
{
    object *o = blitter_new();    
    if (!o) return 0; // error

    // generic attributes
    o->x=x; o->ry=y;o->z=z;

    o->w=(*btc++)*2;
    o->h=(*btc++)*2;

    // palette start + blocks
    o->data = (uint32_t*)btc;
    
    o->line=btc4_2x_line;
    return o;
}

// switch16 version (fastest so far. could be made faster by coding to ASM (?) or blitting 4 lines at a time - full blocks, loading palette progressively ...)
void btc4_2x_line (object *o)  
{
    int line=(vga_line-o->ry)/2; // line into the buffer, zoomed 2x vertically
    uint16_t *palette = (uint16_t*)(o->data);

    // palette is 256 u16 so 128 u32 after start (no padding)
    // data advances width/8 *words* per blockline (and a block is line/4)
    uint32_t *src =  ((uint32_t*)(o->data)) + 128 + (o->w/8)*(line / 4); 

    int x = (o->x) & 0xfffffffe; // ensure word aligned ... case unaligned TBD
    uint32_t *dst = (uint32_t*) (&draw_buffer[x]); 
    int n=o->w/8;

    for (int i=0;i<n;i++) // FIXME should end n accoprding to start x ! 
    {
        uint32_t word_block = *src++; 

        uint32_t c1=palette[ word_block >>24]*0x00010001; // mul is done to repeat twice in the word
        uint32_t c2=palette[(word_block >>16) & 0xff]*0x00010001; 

        switch(word_block>>((line&3)*4) & 0xf) // read two pixels at a time
         {
            case   0 :  *dst++=c2;*dst++ = c2;  *dst++=c2;*dst++=c2; break;
            case   1 :  *dst++=c1;*dst++ = c2;  *dst++=c2;*dst++=c2; break;
            case   2 :  *dst++=c2;*dst++ = c1;  *dst++=c2;*dst++=c2; break;
            case   3 :  *dst++=c1;*dst++ = c1;  *dst++=c2;*dst++=c2; break;
            case   4 :  *dst++=c2;*dst++ = c2;  *dst++=c1;*dst++=c2; break;
            case   5 :  *dst++=c1;*dst++ = c2;  *dst++=c1;*dst++=c2; break;
            case   6 :  *dst++=c2;*dst++ = c1;  *dst++=c1;*dst++=c2; break;
            case   7 :  *dst++=c1;*dst++ = c1;  *dst++=c1;*dst++=c2; break;
            case   8 :  *dst++=c2;*dst++ = c2;  *dst++=c2;*dst++=c1; break;
            case   9 :  *dst++=c1;*dst++ = c2;  *dst++=c2;*dst++=c1; break;
            case  10 :  *dst++=c2;*dst++ = c1;  *dst++=c2;*dst++=c1; break;
            case  11 :  *dst++=c1;*dst++ = c1;  *dst++=c2;*dst++=c1; break;
            case  12 :  *dst++=c2;*dst++ = c2;  *dst++=c1;*dst++=c1; break;
            case  13 :  *dst++=c1;*dst++ = c2;  *dst++=c1;*dst++=c1; break;
            case  14 :  *dst++=c2;*dst++ = c1;  *dst++=c1;*dst++=c1; break;
            case  15 :  *dst++=c1;*dst++ = c1;  *dst++=c1;*dst++=c1; break;
        }
    }
} 
