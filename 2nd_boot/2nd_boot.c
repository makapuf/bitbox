// bootloader

// 2 - second stage bootloader from SD can load a larger bootloader 
// with customizable menu, UI, USB, whatever.


/*
	keep under 64k (error ? )
	cannot intialize USB if already plugged - interactions with bootloader 1 ?
 */

#include <string.h>
#include "stm32f4xx.h" 
#include "system.h" // InitializeSystem
#include "bitbox.h"
#include "flashit.h"

enum {INIT =4, MOUNT=5, OPEN=6, READ=7}; // where we died - bootloader 2
#define MAX_FILES 20
#define LIST_Y 6
#define MSG_X 40
#define MSG_Y 25

void die(int where, int cause);

void led_on() 
{
    GPIOA->BSRRL = 1<<2; 
}

void led_off() 
{
    GPIOA->BSRRH = 1<<2; 
}

void led_init() {
	// init LED GPIO
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // enable gpioA (+)
    GPIOA->MODER |= (1 << 4) ; // set pin 8 to be general purpose output
}

void blink(int times, int speed);

FATFS fs32;
// load from sd to RAM
// flash LED, boot

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


extern const uint8_t font_data [256][16];
char vram_char  [30][80];

int nb_files;
char filenames[MAX_FILES][13]; // 8+3 +. + 1 chr0

#define ICON_W 128 // read from file ?
#define ICON_SIZE (ICON_W*ICON_W/8) // 128x128 1 bit data

int icon_x, icon_y;
uint8_t icon_data[ICON_SIZE]; // 2KB b&w 128x128 data 
// pixel on(i,j) = data[i*16+j/8]&(1<<(7-j%8)))
FIL file;

void print_at(int column, int line, const char *msg)
{
	strcpy(&vram_char[line][column],msg); 
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

void list_roms()
{

    FRESULT res;
    FILINFO fno;
    DIR dir;
    
    char *fn;   /* This function is assuming non-Unicode cfg. */
#if _USE_LFN
    static char lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */
    fno.lfname = lfn;
    fno.lfsize = sizeof lfn;
#endif


    res = f_opendir(&dir, "");                       /* Open the root directory */
    if (res == FR_OK) {
        for (nb_files=0;nb_files<MAX_FILES;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fname[0] == '.') continue;             /* Ignore dot entry */
#if _USE_LFN
            fn = *fno.lfname ? fno.lfname : fno.fname;
#else
            fn = fno.fname;
#endif
            if (!(fno.fattrib & AM_DIR)) { // not a dir                  
            	// assumes non LFN
            	#if _USE_LFN 
            	dslkhgsldkg 
            	#endif

            	// check extension : only keep .bin
            	if (strstr(fn,".BIN")) {
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

}
 
// read icon PBM data to memory
int read_icon(const char *filename)
{
	char c;
	FRESULT res;
	unsigned int b_read;
	
	res = f_open (&file, filename, FA_READ);
	if (res != FR_OK)
		return res;

	// header
	for (int i=0;i<3;i++) {
		res = f_read (&file, &c, 1, &b_read);
		if (res != FR_OK) 
			return res;
		if (c!="P4\n"[i])
			return 1000; // bad header
	}
	 
	// skip one non-comment line  
	int lines=0;
	int cmt=0;
	do {
		res = f_read (&file, &c, 1,&b_read); // read first line char
		cmt = (c=='#'); // asserts no empty line
		while (c!='\n') 
			f_read (&file, &c, 1, &b_read);
			// XXX check size 128
		if (!cmt) lines += 1; // in a comment ?
	} while (lines < 1 || cmt); // skip lines
	 
	res=f_read(&file,icon_data,ICON_SIZE,&b_read);
	f_close(&file);
	return res;
}
 
char *HEX_Digits;

void game_init() {
	flash_init();

	window(2,2,78,4);
	print_at(5,3, " BITBOX bootloader \x01 Hi ! Here are the current files");

    // init FatFS
	memset(&fs32, 0, sizeof(FATFS));
	FRESULT r = f_mount(&fs32,"",1); // mount now
	if (r != FR_OK) {
		print_at(5,20,"Cannot mount disk");
		die(MOUNT,r); 
	}
	
	memset(icon_data, 0x55, sizeof(icon_data));
	icon_x = 400;
	r=read_icon("bitbox.pbm");
	if (r==FR_OK) 
		icon_y=200; 
	else {
		vram_char[16][1] = HEX_Digits[r&0xf];
		vram_char[16][0] = HEX_Digits[(r>>4)&0xf];
		icon_y=1024; // don't display it now
	}

	list_roms();
}


void display_first_event(void)
{
	// display first event, put it back
	struct event e; 
	e=event_get();		
	switch(e.type)
	{
		case evt_keyboard_press : 
			print_at(50,10,"KB pressed      ");
			vram_char[10][61]="0123456789ABCDEF"[e.kbd.key>>4];
			vram_char[10][62]="0123456789ABCDEF"[e.kbd.key&0xf];
		break;

		case evt_keyboard_release : 
			print_at(50,10,"KB released     ");
			vram_char[10][63]="0123456789ABCDEF"[e.kbd.key>>4];
			vram_char[10][64]="0123456789ABCDEF"[e.kbd.key&0xf];
		break;
	}
	if (e.type) event_push(e); // put it back
}

int selected,old_selected=-1;
int x =5,y=10 , dir_x=1, dir_y=1;
char old_val=' ';
char icon_name[13];

void game_frame() 
{
	display_first_event();
	
	// interpret keyboard as gamepad & discard all other events
	kbd_emulate_gamepad();


	// handle input 
	if (GAMEPAD_PRESSED(0,down) && vga_frame%4==0)
		selected +=1;
	if (selected>=nb_files) selected=0;

	if (GAMEPAD_PRESSED(0,up) && vga_frame%4==0)
		selected -=1;
	if (selected<0) selected=nb_files-1;
	
	// XXX try to read game icon if selected is different
	
	if (old_selected != selected)
	{
		strcpy(icon_name, filenames[selected]);
		char *c = strchr(icon_name,'.');
		strcpy(c,".pbm");
		int r=read_icon(icon_name);
		if (r!=FR_OK)
			r=read_icon("bitbox.pbm"); // missed, try standard one
	}
	
	// Start flashing ?
	if (flash_done() && (GAMEPAD_PRESSED(0,start) || GAMEPAD_PRESSED(0,A)))
	{
		print_at(MSG_X,MSG_Y,"Goldorak GO ! Flashing ");
		print_at(MSG_X+24,MSG_Y,filenames[selected]);

		if (f_open(&file,filenames[selected],FA_READ)==FR_OK)
		{
			flash_start_write(&file);
		} else {
			print_at(MSG_X,MSG_Y,"Error reading ");
			print_at(MSG_X+14,MSG_Y,filenames[selected]);
		}		
	}

	memset(&vram_char[MSG_Y+1][MSG_X],' ',30);
	strcpy(&vram_char[MSG_Y+1][MSG_X],flash_message);

	// update_display
	for (int i=0;i<nb_files;i++)
	{
		print_at(10,i+LIST_Y,filenames[i]);		
		// cursor ?
		vram_char[LIST_Y+i][8]=(i==selected)?0x10:' ';
		vram_char[LIST_Y+i][25]=(i==selected)?0x11:' ';
	}

	if (vga_frame%2 == 0 ) {
		// bounce guy
		vram_char[y][x]=old_val;
		if (x==59) dir_x = -1;
		if (x==0)  dir_x = 1;

		if (y==6)  dir_y = 1;
		if (y==29) dir_y = -1;

		x += dir_x;
		y += dir_y;
		old_val = vram_char[y][x];
		vram_char[y][x] = '\x02';
	}

	old_selected=selected;

	flash_frame(); // at the end to let it finish 
}



extern const uint16_t bg_data[256];

void graph_frame() {}

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

	// overdraw icon, 16 bytes data for 128 lines. align icon by 2 pixels.
	if (vga_line>icon_y && vga_line<icon_y+128)
	{
		dst = (uint32_t*) &draw_buffer[icon_x & ~1]; // align 2 pixels
		uint8_t *src = (uint8_t *)&icon_data[(vga_line-icon_y)*16];

		for (int i=0;i<16;i++) // 16*8=128
		{
			// draw a byte worth of pixels
			*dst++ = lut_data[(~*src>>6) & 0x3];
			*dst++ = lut_data[(~*src>>4) & 0x3];
			*dst++ = lut_data[(~*src>>2) & 0x3];
			*dst++ = lut_data[(~*src>>0) & 0x3];

			src++; // next byte
		}
	}
}

void game_snd_buffer(uint16_t *buffer, int len) {} // beeps ?

