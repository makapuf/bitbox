/* simple.h : a simple framebuffer based engine
 * define FRAMEBUFFER_BPP=1,2,4(default),8 to define the framebuffer depth
 */ 

// TODO define window for black borders

#ifndef FRAMEBUFFER_BPP
#define FRAMEBUFFER_BPP 4
#endif 

void clear(); // always clear before first use

extern uint32_t vram[]; // defined as words by example (size depends on BPP settings)

void set_palette(uint8_t c, uint16_t color); // for 8bit kernel, use values <256

void draw_pixel(int x, int y, int c);
void draw_line(int x0, int y0, int x1, int y1, int c);

// void print_at(int column, int line, int color, const char *msg); // font ? 8x8 
