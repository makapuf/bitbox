// bootloader

// 2 - second stage bootloader from SD can load a larger bootloader
// with customizable menu, UI, USB, whatever.


/*
	keep under 64k (error ? )
	cannot intialize USB if already plugged - interactions with bootloader 1 ?
 */

#include <stdbool.h>
#include <stdio.h>

#include <string.h>
#include <stdlib.h> // qsort
// #include "system.h" // system_init
#include "bitbox.h"
#include "fatfs/ff.h"
#include "flashit.h"

char *HEX_Digits="0123456789ABCDEF";


enum {INIT =4, MOUNT=5, OPEN=6, READ=7}; // where we died - bootloader 2
#define MAX_FILES 50
#define DISPLAY_FILENAME_LEN 35
#define FILELIST_X 4

#define SCR_LINES (VGA_V_PIXELS / 16) // Total number of text-lines
#define MSG_X 4

#define TEXT_X 45
#define TEXT_Y 10

#define ICON_W 64 
#define ICON_X 450
#define ICON_Y 80


#if BOARD_PAL
  #define DISPLAY_LINES 12
  #define HEAD_Y 0
  #define MSG_Y 13
  #define LIST_Y 4
#else
  #define DISPLAY_LINES 19
  #define HEAD_Y 2
  #define MSG_Y 25
  #define LIST_Y 6
#endif

FATFS fs32;
// load from sd to RAM
// flash LED, boot


#if 0
#include "stm32f4xx.h"
// Code stolen from "matis"
// http://forum.chibios.org/phpbb/viewtopic.php?f=2&t=338
void jump(uint32_t address)
{
    typedef void (*pFunction)(void);

    pFunction Jump_To_Application;

    // variable that will be loaded with the start address of the application
    uint32_t* JumpAddress;
    const uint32_t* ApplicationAddress = (uint32_t*) address;

    // get jump address from application vector table
    JumpAddress = (uint32_t*) ApplicationAddress[1];

    // load this address into function pointer
    Jump_To_Application = (pFunction) JumpAddress;

    // reset all interrupts to default
    // chSysDisable();

    // Clear pending interrupts just to be on the safe side
    //SCB_ICSR = ICSR_PENDSVCLR;

    // Disable all interrupts
    int i;
    for (i = 0; i < 8; i++)
            NVIC->ICER[i] = NVIC->IABR[i];

    // set stack pointer as in application's vector table
    __set_MSP((u32)(ApplicationAddress[0]));
    Jump_To_Application();
}
#endif 

extern const uint8_t font_data [256][16];
char vram_char[SCR_LINES][80];

int nb_files;
char filenames[MAX_FILES][_MAX_LFN]; // 8+3 +. + 1 chr0

extern const uint16_t bitbox_icon[2+16+ICON_W*ICON_W/4+64];

int icon_x, icon_y;
uint16_t icon_data[sizeof(bitbox_icon)/2-2]; // remove 2 u16 of header
FIL file;

void print_at(int column, int line, const char *msg)
{
	strcpy(&vram_char[line][column],msg);
}

static inline int min (int x, int y)
{
	return x<=y?x:y;
}

// draws an empty window at this position, asserts x1<x2 && y1<y2
// replace with full ascii art ?
void window (int x1, int y1, int x2, int y2 )
{
	for (int i=x1+1;i<x2;i++)
	{
		vram_char[y1][i]='\xCD';
		vram_char[y2][i]='\xCD';
	}
	for (int i=y1+1;i<y2;i++)
	{
		vram_char[i][x1]='\xBA';
		vram_char[i][x2]='\xBA';
	}
	vram_char[y1][x1] ='\xC9';
	vram_char[y1][x2] ='\xBB';
	vram_char[y2][x1] ='\xC8';
	vram_char[y2][x2] ='\xBC';
}


// compare simplt two names
static int cmp(const void *p1, const void *p2){
    return strcmp( (char * const ) p1, (char * const ) p2);
}


void list_roms()
{

    FRESULT res;
    FILINFO fno;
    DIR dir;

    char *fn;   /* This function is assuming non-Unicode cfg. */

    static char lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */
    fno.lfname = lfn;
    fno.lfsize = sizeof lfn;


    res = f_opendir(&dir, ROOT_DIR);                       /* Open the root directory */
    if (res == FR_OK) {
        for (nb_files=0;nb_files<MAX_FILES;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fname[0] == '.') continue;             /* Ignore dot entry */

            // to lowercase
            for (int i=0;i<12;i++) {
            	char *c=&fno.fname[i];
            	if ('A' <= *c && *c <= 'Z') 
            		*c |= 0x40;
            }

            if (strcmp(fno.fname, "2nd_boot.bin") == 0) continue; /* Ignore 2nd boot */

            fn = *fno.lfname ? fno.lfname : fno.fname;

            if (!(fno.fattrib & AM_DIR)) { // not a dir
            	// check extension : only keep .bin
            	if (strstr(fn,".BIN") || strstr(fn,".bin")) { // search ignoring case
            		message("found file %s\n",fn);
                	strcpy(filenames[nb_files],fn);
                	nb_files +=1;
            	}
            }
        }
        if (res != FR_OK) {
	        print_at(MSG_X,MSG_Y,"Error reading directory !");
    	    vram_char[MSG_Y+1][MSG_X] = '0'+res;
        }
        f_closedir(&dir);
    } else {
        print_at(MSG_X,MSG_Y,"Error opening directory !");
        vram_char[MSG_Y+1][MSG_X] = '0'+res;
    }

    // sort it
    qsort(filenames, nb_files, _MAX_LFN, cmp);

}

// read icon data to memory 
/* icon data address in flash is located at 5th interrupt vector.

	u32 header (not put to icon_data)
	u16 colors[16]
	u4  pixel_data[64x64]

	*/
int read_icon(const char *filename)
{
	FRESULT res;
	UINT b_read;
	uint32_t w;

	res = f_open (&file, filename, FA_READ);
	if (res != FR_OK)
		return res;

	// find start offset of icon
	res = f_lseek(&file,7*4 );
	if (res != FR_OK) return res;

	res = f_read (&file, &w, sizeof(w), &b_read);
	if (res != FR_OK) return res;
	message("Icon offset is : %x : %d\n", w, w- 0x08004000);
	if (w==0) return 1000; // no icon, will display default one

	res = f_lseek(&file, w - 0x08004000); // Loadable files are linked to start at 0x08004000
	if (res != FR_OK) return res;

	// header
	res = f_read (&file, &w, sizeof(w), &b_read);
	if (res != FR_OK) return res;
	message("Icon header was 0x%x\n",w);
	if (w != 0x01c0b17b) return 999; // bad header

	res = f_read(&file, &icon_data, sizeof(icon_data), &b_read);	
	
	f_close(&file);
	return res;
}

void set_default_icon( void )
{
	memcpy(icon_data, bitbox_icon+2, sizeof(icon_data)); // +4 : skip header
}


void game_init() {
	flash_init();

	window(2,HEAD_Y,78,HEAD_Y+2);
	print_at(5,HEAD_Y+1, " BITBOX bootloader \x01 Hi ! Here are the current files");

	icon_x = ICON_X;
	icon_y = ICON_Y;

	set_default_icon();

    // init FatFS
	memset(&fs32, 0, sizeof(FATFS));
	FRESULT r = f_mount(&fs32,"",1); // mount now
	if (r != FR_OK) {
		print_at(5,SCR_LINES / 2,"Cannot mount disk");
		die(MOUNT,r);
	}

	list_roms();
	if (!nb_files)
		print_at(MSG_X, MSG_Y, "There are no .bin files on the SD card.");

	print_at(0,SCR_LINES-1,"    Use Gamepad (A or Start to flash) or button (short=next, long=flash)");
}


int selected,old_selected=-1;
int offset=0; // display offset (scrolling)
int x =5,y=10 , dir_x=1, dir_y=1; // guy bouncing 
char old_val=' ';
char icon_name[_MAX_LFN];
bool last_button=false; // button last status
int frame_pressed; // frame when button was pressed


const char* messages[] = {
	[state_idle] 		  = "Awaiting orders ...   ",
	[state_done] 		  = "Done! Now please reset",

	[state_error_erasing] = "Error erasing sector  ", 
	[state_error_reading] = "Error reading data    ", 
	[state_error_writing] = "Error writing sector  ", 
	[state_overflow]      = "Error flash overflow  ",
	[state_unlock_error]  = "Error unlocking Flash ",
 
 	[state_erasing]       = "Erasing sector ..     ", 
 	[state_must_read]     = "Reading sector ..     ",  
	[state_writing]       = "Writing sector ..     ", 
};

void game_frame()
{
	if (vga_frame%2 == 0 ) {
		// bounce guy
		vram_char[y][x]=old_val;
		if (x==59) dir_x = -1;
		if (x==0)  dir_x = 1;

		if (y==HEAD_Y+3)  dir_y = 1;
		if (y==SCR_LINES - 1) dir_y = -1;

		x += dir_x;
		y += dir_y;
		old_val = vram_char[y][x];
		vram_char[y][x] = '\x02';
	}

	if (!nb_files) return; // no need to go further


	// handle button operation
	if (last_button && !button_state()) { // just released 
		if (vga_frame-frame_pressed<120) { // go down
			if (selected<DISPLAY_LINES-1)
				selected +=1;
			else
				offset += 1;

			if (selected+offset >= nb_files) {
				offset=selected=0;
			}
		}
	} else if (button_state() ) { 
		if (!last_button)		// just pressed
			frame_pressed=vga_frame;
		if (vga_frame-frame_pressed>120) {
			// start flashing
			gamepad_buttons[0] |= gamepad_A;
		}
	}
	last_button=button_state();


	// handle input
	if (GAMEPAD_PRESSED(0,down) && vga_frame%4==0 && selected+offset < nb_files-1)
	{
		if (selected<DISPLAY_LINES-1)
			selected +=1;
		else
			offset += 1;
	}


	if (GAMEPAD_PRESSED(0,up) && vga_frame%4==0 && selected+offset > 0 )
	{
		if (selected>0)
			selected -=1;
		else
			offset -= 1;
	}

	if (old_selected != selected) // only if there ARE files to display
	{
		int r=read_icon(filenames[selected]);
		if (r!=FR_OK)
			set_default_icon();
		// display description
		for (int i=0; i<4;i++) {
			// not print_at : not a string, more a char array..
			memcpy(&vram_char[TEXT_Y+i][TEXT_X],(char *)&icon_data[sizeof(icon_data)/2-64+16*i],32);
		}
	}

	// Start flashing ?
	if ((GAMEPAD_PRESSED(0,start) || GAMEPAD_PRESSED(0,A)))
	{
		char *filename = filenames[offset+selected];
		if ( flash_state == state_idle || flash_state == state_done) {
			
			print_at(MSG_X,MSG_Y-1,"Goldorak GO ! Flashing ");
			print_at(MSG_X+24,MSG_Y-1,filename);

			if (f_open(&file,filename,FA_READ)==FR_OK)
			{
				flash_start_write(&file);
			} else {
				print_at(MSG_X,MSG_Y,"Error opening:");
				print_at(MSG_X+14,MSG_Y,filename);
			}
		}
	}


	// update_display
	const int scroll_offset = offset*DISPLAY_LINES/nb_files;
	const int scroll_height = DISPLAY_LINES*DISPLAY_LINES/nb_files;
	for (int i=0;i<min(offset+nb_files, DISPLAY_LINES);i++)
	{
		int l;
		char *s=filenames[offset+i];

		for (l=0;l<DISPLAY_FILENAME_LEN && s[l]!='.';l++)
			vram_char[i+LIST_Y][FILELIST_X+l]=s[l];

		for (;l<DISPLAY_FILENAME_LEN;l++)
			vram_char[i+LIST_Y][FILELIST_X+l]=' ';
		// cursor
		vram_char[LIST_Y+i][FILELIST_X-2]=(i==selected)?0x10:' ';
		vram_char[LIST_Y+i][FILELIST_X+DISPLAY_FILENAME_LEN]=(i==selected)?0x11:' ';

		// scrollbar
		char c = (i>=scroll_offset && i<scroll_offset+scroll_height) ? '\xb1' : '\xb3';
		vram_char[LIST_Y+i][FILELIST_X+DISPLAY_FILENAME_LEN+2] = c;
	}


	old_selected=selected;

	flash_frame(); // at the end to let it finish
	
	// handle flash result

	memset(&vram_char[MSG_Y][MSG_X],' ',40);	
	print_at(MSG_X,MSG_Y,messages[flash_state]); 

	// print extra info
	switch(flash_state) {

		// output sector / bytes
		case state_writing : 
		case state_erasing : 
		case state_must_read : 
			vram_char[MSG_Y][MSG_X+30] = HEX_Digits[current_sector&0xf]; 
			break;

		// output errno
		case state_error_writing :
		case state_error_reading : 
		case state_error_erasing : 
			vram_char[MSG_Y][MSG_X+29] = HEX_Digits[current_sector>>8 &0xf]; 
			vram_char[MSG_Y][MSG_X+30] = HEX_Digits[current_sector&0xf]; 
			set_led(vga_frame & 0x20);
			break;

		// done : draw a sign
		case state_done : 
			window(MSG_X-2,MSG_Y-1,MSG_X+40,MSG_Y+1);
			break;

		// nothing else to do
		case state_unlock_error :
		case state_overflow : 
		case state_idle : 
			break;
	}



}



extern const uint16_t bg_data[256];

void graph_line()
{
	static uint32_t lut_data[4];
	// text mode

	// bg effect, just because
	uint16_t line_color = bg_data[(vga_frame+vga_line)%256];
	lut_data[0] = 0;
	lut_data[1] = line_color<<16;
	lut_data[2] = (uint32_t) line_color;
	lut_data[3] = line_color * 0x10001;


	uint32_t *dst = (uint32_t *) draw_buffer;
	char c;

	for (int i=0;i<80;i++) // column char
	{
		c = font_data[(uint8_t)vram_char[vga_line / 16][i]][vga_line%16];
		// draw a character on this line

		*dst++ = lut_data[(c>>6) & 0x3];
		*dst++ = lut_data[(c>>4) & 0x3];
		*dst++ = lut_data[(c>>2) & 0x3];
		*dst++ = lut_data[(c>>0) & 0x3];
	}

	// overdraw icon, 16 bytes data for 64 lines. align icon by 2 pixels.
	if (vga_line>icon_y && vga_line<icon_y+ICON_W)
	{
		uint16_t *dst16 = &draw_buffer[icon_x & ~1]; // align 2 pixels
		const uint16_t *palette = icon_data;
		uint16_t *src = &icon_data[16+(vga_line-icon_y)*16]; 

		for (int i=0;i<16;i++) // one line =64pixels=16 * 4pixels per u16
		{
			// draw a halfword worth of pixels
			*dst16++ = palette[*src>>12 & 0xf];
			*dst16++ = palette[*src>> 8 & 0xf];
			*dst16++ = palette[*src>> 4 & 0xf];
			*dst16++ = palette[*src>> 0 & 0xf];

			src++; // next byte
		}
	}
}


