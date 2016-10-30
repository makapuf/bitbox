
// --- 16x16 Tilemaps
// --------------------------------------------------------------------------------------

/*
    RAM data :

        *data : start of tilemap
        a : tileset
        b : header
 */
#include "blitter.h"
#include "string.h"

const int tilesizes[] = {16,32,8};

#define HEIGHT_64 64
#define WIDTH_64 64
#define HEIGHT_32 32
#define WIDTH_32 32

#define min(a,b) (a<b?a:b)


#define COPY2 *dst++=*src++; *dst++=*src++;
#define COPY8 COPY2 COPY2 COPY2 COPY2
            /* __asm__ (
                "ldmia %[src]!,{r0-r7} \r\n"
                "stmia %[dst]!,{r0-r7}"
                :[src] "+r" (src), [dst] "+r" (dst)
                :: "r0","r1","r2","r3","r4","r5","r6","r7"
                );
            */

// FIXME factorize much of this (as linlines)

void tilemap_u8_line(object *o)
{
    // in this version, we can assume that we don't have a full

    // use current frame, line, buffer
    unsigned int tilesize = tilesizes[((o->b)>>4)&3];;
    unsigned int tilemap_w = o->b>>20;
    unsigned int tilemap_h = (o->b >>8) & 0xfff;

    o->x &= ~1; // force even addresses ...

    // --- line related
    // line inside tilemap (pixel), looped.
    int sprline = (vga_line-o->ry) % (tilemap_h*tilesize);
    // offset from start of tile (in lines)
    int offset = sprline%tilesize;
    // pointer to the beginning of the tilemap line
    uint8_t *idxptr = (uint8_t *)o->data+(sprline/tilesize) * tilemap_w; // all is in nb of tiles

    // --- column related

    // horizontal tile offset in tilemap
    int tile_x = ((o->x<0?-o->x:0)/tilesize) & (tilemap_w-1);  // positive modulo as tilemap_w is a power of two

    uint32_t * restrict dst = (uint32_t*) &draw_buffer[o->x<0?0:o->x];

    // pixel addr of the last pixel
    const uint32_t *dst_max = (uint32_t*) &draw_buffer[min(o->x+o->w, VGA_H_PIXELS)];

    uint32_t *tiledata = (uint32_t *)o->a;
    uint32_t * restrict src;

    // first, finish first tile (if not on a boundary) , 2 pix at a time
    if (o->x<0 && o->x%tilesize) {
        if (idxptr[tile_x]) {
            src = &tiledata[(idxptr[tile_x]*tilesize + offset)*tilesize*2/4 + (-o->x%tilesize)/2];
            for (int i=0;i<(o->x%tilesize)/2;i++)
                *dst++ = *src++;
        } else { // skip the tile
            dst += (o->x%tilesize)/2; // words per tile
        }
        tile_x++;
    }


    // blit to end of line (and maybe a little more)
    while (dst<dst_max) {
        if (tile_x>=tilemap_w) tile_x-=tilemap_w;

        // blit one tile, 2pix=32bits at a time, 8 times = 16pixels, 16 times=32pixels
        if (idxptr[tile_x]) {
            src = &tiledata[(idxptr[tile_x]*tilesize + offset)*tilesize*2/4];

            for (int i=0;i<4;i++) *dst++=*src++; // 4 words = 8pixels
            if (tilesize>=16) // 16 or 32
                for (int i=0;i<4;i++) *dst++=*src++; // 8 more
            if (tilesize==32)
                for (int i=0;i<8;i++) *dst++=*src++; // 16 more

        } else { // skip the tile
            dst += tilesize/2; // words per tile
        }
        tile_x++;

    }

}


void tilemap_u16_line(object *o)
{
    // in this version, we can assume that we don't have a full
    // TODO : take care of smaller x, don't recalc all each time. case o->x <0
    // TODO handle tilesize=8

    // use current frame, line, buffer
    unsigned int tilesize = tilesizes[((o->b)>>4)&3];;
    unsigned int tilemap_w = o->b>>20;
    unsigned int tilemap_h = (o->b >>8) & 0xfff;

    o->x &= ~1; // force even addresses ...

    // --- line related
    // line inside tilemap (pixel), looped.
    int sprline = (vga_line-o->ry) % (tilemap_h*tilesize);
    // offset from start of tile (in lines)
    int offset = sprline%tilesize;
    // pointer to the beginning of the tilemap line
    uint16_t *idxptr = (uint16_t *)o->data+(sprline/tilesize) * tilemap_w; // all is in nb of tiles

    // --- column related

    // horizontal tile offset in tilemap
    int tile_x = ((o->x<0?-o->x:0)/tilesize)&(tilemap_w-1);  // positive modulo
    // positive modulo : i&(tilemap_w-1) if tilemap size is a power of two

    uint32_t * restrict dst = (uint32_t*) &draw_buffer[o->x<0?0:o->x];

    // pixel addr of the last pixel
    const uint32_t *dst_max = (uint32_t*) &draw_buffer[min(o->x+o->w, VGA_H_PIXELS)];

    uint32_t *tiledata = (uint32_t *)o->a;
    uint32_t * restrict src;

    // first, finish first tile, 2 pix at a time
    if (o->x<0) {
        if (idxptr[tile_x]) {
            src = &tiledata[(idxptr[tile_x]*tilesize + offset)*tilesize*2/4 + (-o->x%tilesize)/2];
            for (int i=0;i<(o->x%tilesize)/2;i++)
                *dst++ = *src++;
        } else { // skip the tile
            dst += tilesize/2; // words per tile
        }
        tile_x++;
    }

    // blit to end of line (and maybe a little more)
    while (dst<dst_max) {
        if (tile_x>=tilemap_w) tile_x-=tilemap_w;

        // blit one tile, 2pix=32bits at a time, 8 times = 16pixels, 16 times=32pixels
        if (idxptr[tile_x]) {
            src = &tiledata[(idxptr[tile_x]*tilesize + offset)*tilesize*2/4];

            for (int i=0;i<4;i++) *dst++=*src++; // 4 words = 8pixels
            if (tilesize>=16) // 16 or 32
                for (int i=0;i<4;i++) *dst++=*src++; // 8 more
            if (tilesize==32)
                for (int i=0;i<8;i++) *dst++=*src++; // 16 more

        } else { // skip the tile
            dst += tilesize/2; // words per tile
        }
        tile_x++;

    }

}

void tilemap_u8_line8(object *o)
{
    // simplified a bit
    const uint8_t *buffer8 = (uint8_t*) draw_buffer;

    // use current frame, line, buffer
    unsigned int tilesize = tilesizes[((o->b)>>4)&3];
    unsigned int tilemap_w = o->b>>20;
    unsigned int tilemap_h = (o->b >>8) & 0xfff;

    o->x &= ~3; // force addresses mod4

    // --- line related
    // line inside tilemap (pixel), looped.
    int sprline = (vga_line-o->ry) % (tilemap_h*tilesize);
    // offset from start of tile (in lines)
    int offset = sprline%tilesize;
    // pointer to the beginning of the tilemap line
    uint8_t *idxptr = (uint8_t *)o->data+(sprline/tilesize) * tilemap_w; // all is in nb of tiles

    // --- column related -> in frame ?

    // horizontal tile offset in tilemap
    int tile_x = ((o->x<0?-o->x:0)/tilesize)&(tilemap_w-1);  // positive modulo
    // positive modulo : i&(tilemap_w-1) if tilemap size is a power of two

    uint8_t * restrict dst = (uint8_t*) &buffer8[o->x<0?0:o->x];

    // pixel addr of the last pixel
    const uint8_t *dst_max = (uint8_t*) &buffer8[min(o->x+o->w, VGA_H_PIXELS)];

    uint8_t *tiledata = (uint8_t *)o->a; // nope : read 4 indices at once.
    uint8_t *restrict src;  // __builtin_assume_aligned

    // blit to end of line (and maybe a little more)
    // we needed to loop over

    // XXX specify tilesize ?

    while (dst<dst_max) {
        if (tile_x>=tilemap_w)
            tile_x-=tilemap_w;

        // blit one tile, 2pix=32bits at a time, 8 times = 16pixels, 16 times=32pixels
        if (idxptr[tile_x]) {
            src = &tiledata[(idxptr[tile_x]*tilesize + offset)*tilesize];

            // Verify out of the loop, fix tilesize ?
            // assume aligned.
            memcpy(dst,src,tilesize);

        };
        dst += tilesize; // words per tile
        tile_x++;
    }
}



object *tilemap_new(const void *tileset, int w, int h, uint32_t header, const void *tilemap)
{
    object *o = blitter_new();
    if (!o) return 0; // error

    o->data = (uint32_t*)tilemap;

    int tilesize = tilesizes[(header>>4)&3];

    o->b = header;

    // generic attributes
    // 0 for object width == tilemap width
    o->w=w?w:(header>>20)*tilesize;
    o->h=h?h:((header>>8)&0xfff)*tilesize;

    o->x=o->y=0;
    o->z = 100; // a little behind sprites by default


    #if VGA_BPP==8 // 8-bit interface

    if (!(header & TSET_8bit)) {
        die(4,5);
    }
    if ((header&0xf) != TMAP_U8) {
        die(4,6); // not implemented
    }

    o->a = (uintptr_t)tileset-tilesize*tilesize; // to start at index 1 and not 0, offset now in bytes.
    o->line = tilemap_u8_line8;

    #else // 16-bit interface

    if ((header & TSET_8bit)) {
        message("Error: 8bit tileset on a 16bit screen ?");
        die(4,7);
    }

    o->a = (uintptr_t)tileset-2*tilesize*tilesize; // to start at index 1 and not 0, offset now in bytes.
    switch (header & 0xf) {
        case TMAP_U8 :
            o->line=tilemap_u8_line;
            break;
        case TMAP_U16 :
            o->line=tilemap_u16_line;
            break;
        default:
            die(4,4);
            break;
    }
    #endif

    return o;
}

// blit a tmap inside another one.
// must have same type of maps (and same tileset is often preferable)
void tmap_blit(object *tm, int x, int y, uint32_t src_header, const void *data)
{
    int src_w = (src_header>>20);
    int src_h = ((src_header>>8) & 0xfff);

    int src_type = src_header & 0xff; // size+type part

    uint32_t dst_header = tm->b;
    int dst_w = (dst_header>>20);
    int dst_h = ((dst_header>>8) & 0xfff);
    int dst_type = dst_header & 0xff;

    // XXX FIXME handle different cases ?
    if (dst_type != src_type) {
        message ("Error blitting tmap : dst type : %d, src type %d\n", src_type, dst_type);
        die(5,5);
    }

    for (int j=0;j<src_h && j<(dst_h-y);j++)
        for (int i=0;i<src_w && i<(dst_w-x);i++) {
            if ((dst_type & 0x0f )==TMAP_U8)  { // only consider tmap type
                uint8_t c = ((uint8_t*)data) [src_w * j+i ];
                if (c) ((uint8_t *)tm->data) [(j+y)*dst_w+i+x] = c;
            } else {
                uint16_t c = ((uint16_t*) data)[src_w * j+i ];
                if (c) ((uint16_t *)tm->data)[(j+y)*dst_w+i+x] = c;
            }
        }
}


void tmap_blitlayer(object *tm, int x, int y, uint32_t src_header, const void* data, int layer)
{
    int src_w = (src_header>>20);
    int src_h = ((src_header>>8) & 0xfff);

    tmap_blit(tm,x,y,src_header,data+src_w*src_h*layer);
}