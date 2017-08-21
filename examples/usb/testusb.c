/* Test for USB devices and input events
 * Copyright 2014, Adrien Destugues <pulkomandy@pulkomandy.tk>
 * 2016 Makapuf (split event / non events)
 */

#include <string.h>

#include "bitbox.h"
#include "lib/textmode/textmode.h"

// graphical gamepad position 
#define PAD_X 2
#define PAD_Y 10
#define PAD_Y2 15
#define KB_Y 23 // keyboard press positions

int cx, cy;

// draw "graphical" controller frame
void draw_controller(int x, int y)
{
	window(0,x, y, x+17, y+4);
	print_at(x+2 ,y,0,"[ ]");
	print_at(x+13,y,0,"[ ]");
}

void update_controller (int x, int y, uint16_t buttons)
{
	vram[y+1][x+3] = buttons & gamepad_up ? 'U':'u';
	vram[y+3][x+3] = buttons & gamepad_down ? 'D':'d';
	vram[y+2][x+1] = buttons & gamepad_left ? 'L':'l';
	vram[y+2][x+5] = buttons & gamepad_right ? 'R':'r';

	vram[y+2][x+16] = buttons & gamepad_A ? 'A':'a';
	vram[y+3][x+14] = buttons & gamepad_B ? 'B':'b';
	vram[y+1][x+14] = buttons & gamepad_X ? 'X':'x';
	vram[y+2][x+12] = buttons & gamepad_Y ? 'Y':'y';

	vram[y  ][x+ 3] = buttons & gamepad_L ? 'L':'l';
	vram[y  ][x+14] = buttons & gamepad_R ? 'R':'r';
	vram[y+3][x+ 9] = buttons & gamepad_select ? 'S':'s';
	vram[y+3][x+10] = buttons & gamepad_start ? 'G':'g';
}

void printhex(int x, int y, uint8_t n)
{
	static const char *HEX_Digits = "0123456789ABCDEF";
	vram[y][x]=HEX_Digits[(n>>4) & 0xF];
	vram[y][x+1]=HEX_Digits[n&0xf];
}

void printhex16(int x, int y, uint16_t n)
{
	static const char *HEX_Digits = "0123456789ABCDEF";
	for (int i=0;i<4;i++) {
		vram[y][x+3-i]=HEX_Digits[(n>>(4*i)) & 0xF];
	}
}



// lowlevel USB 

#ifdef USE_USB_OTG_FS 
#include "usbh_core.h"
extern USBH_HOST USB_FS_Host;
extern USB_OTG_CORE_HANDLE USB_OTG_FS_Core;

#ifdef USE_USB_OTG_HS
extern USBH_HOST USB_Host;
extern USB_OTG_CORE_HANDLE USB_OTG_Core;
#endif 

#endif 

const char *host_state_str[] = {
	"Idle      ",	"Dev Attach",	"Dev Discon",	"Detect Spd",
	"Enum      ",	"Class req.",	"Class     ",	"Ctrl Xfer ",
	"Usr Input ",	"Suspended ",	"Error     ",
	// extra for emulation, shouldn't be used on device
};

const char *host_ctrl_state_str[]={
  "Idle           ",
  "Setup          ",
  "Setup Wait     ",
  "Data In        ",
  "Data In Wait   ",
  "Data Out       ",
  "Data Out Wait  ",
  "Status In      ",
  "Status In_wait ",
  "Status Out     ",
  "Status Out Wait",
  "Error          ",
  "Stalled        ",
  "Complete       ",
};

const char *host_speeds[]={"High","Full","Low","---"};

void update_host_status(void)
{

	#ifdef USE_USB_OTG_HS 
	/*const USB_OTG_GOTGCTL_TypeDef *gotgctl_hs = (USB_OTG_GOTGCTL_TypeDef*) \
		&USB_OTG_Core.regs.GREGS->GOTGCTL;*/
	const USB_OTG_HPRT0_TypeDef *hprt_hs = (USB_OTG_HPRT0_TypeDef*) \
		USB_OTG_Core.regs.HPRT0;

	//print_at(23,30,0,gotgctl_hs->b.asesvld ? "Yes":"No ");
	print_at(23,30,0,hprt_hs->b.prtena ? "Yes":"No ");
	print_at(23,31,0,hprt_hs->b.prtconnsts ? "Yes":"No ");
	print_at(23,32,0,host_speeds[hprt_hs->b.prtspd]);
	print_at(23,33,0,host_state_str[USB_Host.gState]);
	print_at(23,34,0,host_ctrl_state_str[USB_Host.Control.state]);

	printhex16(23,35,USB_OTG_Core.nbpackets_in);
	printhex16(28,35,USB_OTG_Core.nbpackets_out);
	printhex16(23,36,USB_OTG_Core.nbbytes_in);
	printhex16(28,36,USB_OTG_Core.nbbytes_out);

	#endif 

	#ifdef USE_USB_OTG_FS 
	/*const USB_OTG_GOTGCTL_TypeDef *gotgctl_fs = (USB_OTG_GOTGCTL_TypeDef*) \
		&USB_OTG_FS_Core.regs.GREGS->GOTGCTL;*/
	const USB_OTG_HPRT0_TypeDef *hprt_fs = (USB_OTG_HPRT0_TypeDef*) \
		USB_OTG_FS_Core.regs.HPRT0;

	//print_at(32,30,0,gotgctl_fs->b.asesvld ? "Yes":"No ");
	print_at(33,30,0,hprt_fs->b.prtena ? "Yes":"No ");
	print_at(33,31,0,hprt_fs->b.prtconnsts ? "Yes":"No "); 
	print_at(33,32,0,host_speeds[hprt_fs->b.prtspd]);
	print_at(33,33,0,host_state_str[USB_FS_Host.gState]);
	print_at(33,34,0,host_ctrl_state_str[USB_FS_Host.Control.state]);

	printhex16(33,35,USB_OTG_FS_Core.nbpackets_in);
	printhex16(38,35,USB_OTG_FS_Core.nbpackets_out);
	printhex16(33,36,USB_OTG_FS_Core.nbbytes_in);
	printhex16(38,36,USB_OTG_FS_Core.nbbytes_out);

	#endif 

}


char *KBMOD="CSAWCSAW";

void game_init() {
	clear(); 
	set_palette(1,RGB(255,255,160),0); // light yellow on black

	// testing vga bits on micro
	for (int i=0;i<8;i++) {
		set_palette(2+i,1<<(7-i),0);
		print_at(20+i,0,2+i,"\x7");
	}

	
	window(0,2,1,45,4);
	print_at(14,2,1, " \xf9\xfa\xfb USB TEST ");
	print_at(5,3,0,  " \x01 Hi ! Plug some usb device...");
	print_at(2, 6, 1,"Mouse:");
	print_at(9, 6, 0,"X=   Y=   lmr");

	print_at(2,PAD_Y-2,1, "Gamepads:");
	draw_controller(PAD_X, PAD_Y);
	draw_controller(PAD_X, PAD_Y2);

	// analog values
	print_at(27,PAD_Y-2,1,"Analog pad0:");
	window (0,27, PAD_Y, 27+17, PAD_Y+9);

	// keyboard 
	print_at(2,KB_Y,1,"Keyboard:");

	// lowlevel
	print_at( 2,28,1,"USB Low level:");
	print_at(22,29,0,"[0-HS]   [1-FS]");
	print_at( 2,30,0,"      Port enabled");
	print_at( 2,31,0,"  Device connected");
	print_at( 2,32,0,"      Device speed");
	print_at( 2,33,0,"       Host status");
	print_at( 2,34,0,"   Host Ctrl State");
	print_at( 2,35,0,"  Transfers In/Out");
	print_at( 2,36,0,"  Data Size In/Out");


	cx = cy = 0;
}
void game_frame() 
{
	static char cbak;
	static int8_t gpx, gpy;

	// mouse
	vram[cy / 8][cx / 8] = cbak;

	cy += mouse_y; mouse_y=0;
	if (cy < 0) cy = VGA_V_PIXELS;
	else if (cy >= VGA_V_PIXELS) cy = 0;
	
	cx += mouse_x; mouse_x=0;
	if (cx < 0) cx = VGA_H_PIXELS;
	else if (cx >= VGA_H_PIXELS) cx = 0;

	printhex(11,6,cx/8);
	printhex(16,6,cy/8);

	cbak = vram[cy / 8][cx / 8];
	vram[cy / 8][cx / 8] = 127;

	vram[6][19]=mouse_buttons & mousebut_left?'L':'l';
	vram[6][20]=mouse_buttons & mousebut_middle?'M':'m';
	vram[6][21]=mouse_buttons & mousebut_right?'R':'r';

	printhex(23,6,mouse_buttons);

	// gamepad buttons
	update_controller(PAD_X, PAD_Y,gamepad_buttons[0]);
	update_controller(PAD_X, PAD_Y2,gamepad_buttons[1]);


	// analog gamepad
	printhex(40,PAD_Y-2,gamepad_x[0]);
	printhex(43,PAD_Y-2,gamepad_y[0]);

	vram[15 + gpy / 32][36  + gpx / 16] = ' ';

	gpx = gamepad_x[0];
	gpy = gamepad_y[0];
	vram[15 + gpy / 32][36 + gpx / 16] = '+';

	// KB codes 
	for (int i=0;i<6;i++) {
		printhex(5+i*3,KB_Y+2,keyboard_key[0][i]);
	}

	// KB mods
	for (int i=0;i<8;i++)
		vram[KB_Y+3][5+i]=keyboard_mod[0] & (1<<i) ? KBMOD[i] : '-' ;


	// low level stuff
	update_host_status();

}

