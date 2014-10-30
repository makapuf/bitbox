
// --- 16x16 Tilemaps
// --------------------------------------------------------------------------------------

/*
    RAM data : 
    
        *data : start of tilemap 
        a : tileset
        b : header
 */
#include "blitter.h"


#define TILESIZE16 16
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

// TODO : integrate as one same line ?
void tilemap_6464u8_line(object *o)
{
    // in this version, we can assume that we don't have a full 
    // TODO : take care of smaller x, don't recalc all each time. case o->x <0 

    // use current frame, line, buffer

    // line inside tile (pixel), looped.
    int sprline = (vga_line-o->ry) % (HEIGHT_64*16); 

    // index of first tile to run in tile map, in pixels
    int start_tile = o->x%(WIDTH_64*16)/16; 

    // which tile to start on 
    uint8_t *idxptr = (uint8_t *)o->data+(sprline/TILESIZE16) * WIDTH_64 + start_tile; // all is in nb of tiles

    // offset from start of tile (in lines)
    int offset = sprline%TILESIZE16; 

    // printf("idxptr : %x base : %x, 1st ref = %d\n",idxptr, ot->frames, *idxptr);

    uint32_t * restrict dst = (uint32_t*) &draw_buffer[o->x+start_tile*TILESIZE16]; 
    int right_stop = min(o->x+o->w, VGA_H_PIXELS); // end at which pixel ? 

    const uint32_t *dst_max = (uint32_t*) &draw_buffer[right_stop]; // pixel addr of the last pixel
    // dst = (uint32_t*) ((uint32_t) dst & ~1); // FAULT if x is odd ?
    
    uint32_t *tiledata = (uint32_t *)o->a;

    while (dst<dst_max) 
    {
        // blit one tile, 2pix=32bits at a time, 8times = 16pixels
        if (*idxptr) {
            uint32_t * restrict src;
            src = &tiledata[((*idxptr)*TILESIZE16 + offset)*TILESIZE16*2/4];  

            // force unroll
            COPY8;

        } else { // skip the tile
            dst += TILESIZE16/2; // words per tile
        }
        idxptr++;
    }
}

void tilemap_3232u8_line(object *o)
{
    // TODO : take care of smaller x, don't recalc all each time. case o->x <0 

    // use current frame, line, buffer

    // line inside tile (pixel), looped.
    int sprline = (vga_line-o->ry) % (HEIGHT_32*16); 

    // index of first tile to run in tile map, in pixels
    int start_tile = o->x%(WIDTH_32*16)/16; 

    // which tile to start on 
    uint8_t *idxptr = (uint8_t *)o->data+(sprline/TILESIZE16) * WIDTH_32 + start_tile; // all is in nb of tiles

    // offset from start of tile (in lines)
    int offset = sprline%TILESIZE16; 

    // printf("idxptr : %x base : %x, 1st ref = %d\n",idxptr, ot->frames, *idxptr);

    uint32_t * restrict dst = (uint32_t*) &draw_buffer[o->x+start_tile*TILESIZE16]; 
    int right_stop = min(o->x+o->w, VGA_H_PIXELS); // end at which pixel ? 

    const uint32_t *dst_max = (uint32_t*) &draw_buffer[right_stop]; // pixel addr of the last pixel
    // dst = (uint32_t*) ((uint32_t) dst & ~1); // FAULT if x is odd ?
    
    uint32_t *tiledata = (uint32_t *)o->a;

    while (dst<dst_max) 
    {
        // blit one tile, 2pix=32bits at a time, 8times = 16pixels
        if (*idxptr) {
            uint32_t * restrict src;
            src = &tiledata[((*idxptr)*TILESIZE16 + offset)*TILESIZE16*2/4];  

            // force unroll
            COPY8;

        } else { // skip the tile
            dst += TILESIZE16/2; // words per tile
        }
        idxptr++;
    }
}



object * tilemap_new(const uint16_t *tileset, int w, int h, uint32_t header, void *tilemap)
{
    object *o = blitter_new();    
    if (!o) return 0; // error


    // generic attributes
    o->w=w;  o->h=h;

    o->data = (uint32_t*)tilemap; 
    o->a = (uintptr_t)tileset-512; // to start at index 1 and not 0, offset now in bytes.
    o->b = header;

    switch (header) {
        case TILEMAP_6464u8 :
            o->line=tilemap_6464u8_line;
            break;

        case TILEMAP_3232u8 :
            o->line=tilemap_3232u8_line;
            break;

        default :
            message("Unknown format code in tileset!");
            return 0; // error : not implemented
            break;
    }
    return o;
}

// blit a tmap inside another one.
// must have same type of maps (and same tileset is often preferable)
void tmap_blit(object *tm, int x, int y, uint32_t src_header, const void *data)
{
    int src_w = (src_header>>24);
    int src_h = ((src_header>>16) & 0xff);
    int src_type = src_header & 0xff;

    uint32_t dst_header = tm->b;
    int dst_w = (dst_header>>24);
    int dst_h = ((dst_header>>16) & 0xff);
    int dst_type = dst_header & 0xff;

    // XXX FIXME handle different cases ?
    if (dst_type != src_type) {
        message ("Error blitting tmap : dst type : %d, src type %d\n", src_type, dst_type);
        die(5,5);
    }

    for (int j=0;j<src_h && j<(dst_h-y);j++)
        for (int i=0;i<src_w && i<(dst_w-x);i++) {
            if (dst_type==TMAP_U8)  {
                uint8_t c = ((uint8_t*)data) [src_w * j+i ]; // skips 4 u8
                if (c) ((uint8_t *)tm->data) [(j+y)*dst_w+i+x] = c;                
            } else {
                uint16_t c = ((uint16_t*) data)[src_w * j+i ]; // skips 2 u16
                if (c) ((uint16_t *)tm->data)[(j+y)*dst_w+i+x] = c;                
            }
        }
}
