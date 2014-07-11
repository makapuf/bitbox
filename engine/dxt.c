/* Decoding DXT data in memory 

    DXT data is stored as a list of blocks
    extra palette data is stored as a list of 16 bits palette data 

*/

// TODO : FIX input DXT as 15 bits to allow direct blits / recalc all a 15bits
    // +get rid of 15bit data

#include <stdint.h>
    
//#include <core_cm4_simd.h>

uint32_t __SMUAD(uint32_t a, uint32_t b)
// Signed dual signed multiply, add (no accumulate), top*top + bottom*bottom
{
    int16_t a0,a1,b0,b1;
    a0 = a>>16; // endianness
    a1 = a&0xffff;
    b0 = b>>16; // endianness
    b1 = b&0xffff;

    return a0*b0+a1*b1;
}

uint32_t __UQADD16(uint32_t a,uint32_t b)
{
    int16_t a0,a1,b0,b1;
    // add top/bottom (with sat16), should keep low
    a0 = a>>16; // endianness
    a1 = a&0xffff;
    b0 = b>>16; // endianness
    b1 = b&0xffff;
    return (a0+b0)<<16 | (a1+b1); // XXX SAT
}



typedef struct {
    uint16_t c[2]; // colors, rrrrrggggggbbbbb : high bit MUST be consistent (used in c0 <> c1)
    // if c0>c1, c2=2/3 c0+1/3 c1 ; c3 = invert . if c0<=c1 : c2=1/2 c0 + 1/2 c1, c3=transparent
    uint8_t pixels[4]; // 2bits/pixel, line by line, 1 byte per line.
}  block; // no padding 

typedef struct 
{
    unsigned int w,h;
    block blocks[];
} DXTFile;


void dxt_decode_palette(block *blocks, uint16_t *palette, unsigned int nb_blocks)
/* decode a line of palette [c2, c3, ..] from DXT data. */
// XXX dont do it all if const pixels ! 
{
    // extract to 0xrrrrrggggggbbbbb to rgb fixed 8.8

    uint32_t *pal32=(uint32_t*) palette; // view as couples

    const int16_t k1_3 = 256/3; // XXX 265 better than 256 !!  - mais sature le rouge sauf si 3e au lieu de 3f pr vert ???
    const int16_t k2_3 = 2*256/3;

    uint32_t c01,r01,g01,b01,r23,g23,b23,c23;

    for (int i=0;i<nb_blocks;i++)
    {
        
        // c= rrrrrggg_gggbbbbb  rrrrrggg_gggbbbbb 
        // -> 000rrrrr_00000000  000rrrrr_00000000 (XXX ! pourrait etre rrrrr_rrrrr111)
        // -> 000ggggg_g0000000  000ggggg_g0000000 (1 bit apres la virgule)
        // -> 000bbbbb_00000000  000bbbbb_00000000


        
        c01 = (blocks[i].c[0])<<16 | blocks[i].c[1]; // MSB / LSB, endianness 
        r01 = (c01 & (0x1f<<11 | 0x1f<<(11+16) ))>>3;
        g01 = (c01 & (0x3f<<5  | 0x3f<<( 5+16) ))<<2;
        b01 = (c01 & (0x1f     | 0x1f<<( 0+16) ))<<8;

        // slightly better, a bit slower
        // 000rrrrr00000000 ->  000rrrrr_rrrrr111 : (r>>11)*0x0108
        r01 = r01 | r01 >>5;
        g01 = g01 | g01 >>5;
        b01 = b01 | b01 >>5;


        if (blocks[i].c[0]>blocks[i].c[1]) 
        {
            uint32_t r2,r3,g2,g3,b2,b3;

            // Signed dual signed multiply, add (no accumulate), top*top + bottom*bottom (1cycle) -> 32bits
            /*
            r2 = r0*k1_3 + r1*k2_3 = r01 *+ (1/3,2/3); (prendre le high byte)
            r3 = r0*k2_3 + r1*k1_3;
            ...
            */

            r2=__SMUAD(r01, k1_3<<16|k2_3); // 00 rr xx xx
            r3=__SMUAD(r01, k2_3<<16|k1_3);

            g2=__SMUAD(g01, k1_3<<16|k2_3);
            g3=__SMUAD(g01, k2_3<<16|k1_3);

            b2=__SMUAD(b01, k1_3<<16|k2_3);
            b3=__SMUAD(b01, k2_3<<16|k1_3);

            // -> 00000000_000rrrrr 00000000_000rrrrr
            // -> 00000000_000ggggg 00000000_000ggggg 
            // -> 00000000_000bbbbb 00000000_000bbbbb
            r23 = r2&0xffff0000 | r3>>16;
            g23 = g2&0xffff0000 | g3>>16;
            b23 = b2&0xffff0000 | b3>>16;

            // repack 
            // c= 0rrrrrgg_gggbbbbb  0rrrrrgg_gggbbbbb (ou 1xxxxxxx xxxxxxx pour transp)
            c23 = r23 <<10 | g23 <<5 | b23 ; // & (0x1f | 0x1f << 16)) ?
        } 
        else // transparent case
        {
            // add top/bottom (with sat16), should keep low hw because dont care about c2
            r23=__UQADD16(r01,r01>>16)&0xfe00; // sum on byte 1 as 00rrrrrr_00000000, del low bit 00rrrrr0_00000000
            g23=__UQADD16(g01,g01>>16)&0xfe00; //  
            b23=__UQADD16(b01,b01>>16)&0xfe00; // & not used  because of >> after

            // pack & place low (ie will be sent in first u16)
            c23 = r23<<1 | g23>>4 | b23>>9 | 0x80000000;  // set c3 high bit for transp.
        }

        *pal32++ =  c23;
    }
}



void dxt_decompr_line_notrans(block *blocks, uint16_t *dst, unsigned int nb_blocks, uint16_t *palette, unsigned int line)
/* decompress 1 line of blocks.
    Don't take transparency into account (faster), assumes opaque

    src : blocks
    dest : pixel data (at least 4*nb blocks), aligned by word.
    nb blocks : number of 4 pixels blocks to decompress
    palette :  same as blocks, 2 hw per blocks (update palette before !)
    line : line offset (0-3) 

    XXX SPEEDUP : transfer two pixels by two (ie nibble-> 2 pixels, palette 4->16) & blit 2 pixels directly. needs alignment.

    // preparer palettes & *dst++=palette[c&3] - plus rapide encore
*/
    {}
;

static inline uint16_t c1615(uint16_t x)
{
    return (x&0b1111111111000000)>>1 | (x&0b11111);
}

void dxt_decompr_line(block *blocks, uint16_t *dst, unsigned int nb_blocks, uint16_t *palette,unsigned int line)
/* decompress 1 line of blocks.

    blocks : blocks of DXT data
    dest : pixel data (at least 4*nb blocks)
    nb blocks : number of 4 pixels blocks to decompress
    palette :  same as blocks, 2 hw per blocks (update palette before !) - computed palette only
    line : line offset in block (0-3) 

    XXX speedup : check 0000 lines ? (check used)
*/
{
    uint8_t pixels;
    uint16_t color;

    for (int i=0;i<nb_blocks;i++, palette+=2)
    {
        pixels = blocks[i].pixels[line]; 
        //printf("pal : %04x %04x %04x %04x\n",blocks[i].c[0],blocks[i].c[1],*palette,*(palette+1));

        // fast path if constant XXX test if useful (ie tjs au moins un ??)
        // quick test to check if 4 x 2bits ? (a*0x10101010^0x0055AAFF)+check one byte null?
        
        switch ((pixels))
        {
            case 0b00000000 : 
                color = c1615(blocks[i].c[0]); 
                for (int pix=0;pix<4;pix++) 
                    *dst++=color; 
                break;

            case 0b01010101 : 
                color = c1615(blocks[i].c[1]);
                for (int pix=0;pix<4;pix++) 
                    *dst++=color;
                break;

            case 0b10101010 : 
                color = *palette;   
                for (int pix=0;pix<4;pix++) 
                    *dst++=color; 
                break;

            case 0b11111111 : 
                // test if high bit 1 of palette2 is transparent
                if (!(*(palette+1)&0x8000)) 
                {
                    color = *(palette+1); 
                    for (int pix=0;pix<4;pix++) 
                        *dst++=color;
                }
                break; 

            default : // other cases
                // start from highest bits.
                // XXX check Unroll !
                for (int pix=0;pix<4;pix++, pixels >>=2) 
                {
                    switch ((pixels & 0x3))
                    {
                        case 0 : *dst++ = c1615(blocks[i].c[0]); break;
                        case 1 : *dst++ = c1615(blocks[i].c[1]); break;
                        case 2 : *dst++ = *palette;    break;
                        // test if high bit 1 of palette is transparent
                        case 3 : if (!(*(palette+1)&0x8000)) 
                            *dst++ = *(palette+1); 
                        break; 
                    }
                } 
                 
                break; 
        } // switch
        
    } // for blocks
}




