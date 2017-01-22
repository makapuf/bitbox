CHANGELOG.md

# v0.10 

### new features
- added serial library 

### breaking changes:

- split libraries to /lib and kernel to /kernel, sent demos & scripts to their lib. 
	Porting : 
	    to port from old kernel, move your /lib references to /kernel (mainly bitbox.mk)

- cleaned the VGAMODE_XXX into one setting : VGA_MODE= 320,400,640, ...
	Porting : 
	    change VGAMODE_XXX to VGA_MODE=XXX

- make explicit VGA_BPP=8 or 16 (& adapt libraries) - now draw_buffer is defined as 8bit or 16 bits throught a pixel_t type, also RGB macro now defined differently
	Porting : 
	    default is 16bits, so no impact. If you use a 8bit kernel, specify it

- split simple modes to textmode and framebuffer libs, make it adapt to current VGA MODE instead of defining it
	Porting : 
	    now include textmode.c or framebuffer.c explicitly in your makefile CFILES and define BPP or FONT 

- changed graph_frame callbacks to graph_vsync optional callback called in each vsync line 
	Porting : 
	    quickest (if you define a non-empty graph_frame) 
	    is to change graph_frame to graph_vsync and add a line like: 

	        if (vga_line!=VGA_V_PIXELS) return; 

- put events in its own library and removed from bitbox. Now only state variables are handled by kernel, events are handled by 
	porting : 
	    if using events, use events library.
	    if using gamepad or keyboard as a gamepad : 
	    no changes, except kbd_emulate_gamepad() is no needed anymore

### Fixes : 
- fix PIL imports in scripts (broken Travis)
