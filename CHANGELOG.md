CHANGELOG.md

# v0.2 

### new features
- added serial library 

### breaking changes:
- split libraries to /lib and kernel to /kernel, sent demos & scripts to their lib
- cleaned the VGAMODE_XXX into one setting : VGA_MODE= 320,400,640, ...
- make explicit VGA_BPP=8 or 16 (& adapt libraries) - now draw_buffer is defined as 8bit or 16 bits throught a pixel_t type, also RGB macro now defined differently
- split simple modes to textmode and framebuffer libs, make it adapt to current VGA MODE instead of defining it
- changed graph_frame callbacks to graph_vsync optional callback called in each vsync line 

### Fixes : 
- fix PIL imports (broken Travis)
