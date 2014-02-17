#include <stdint.h>
// defs for second sector = EEPROM
#define MAX_RECORDS 4096


// erase block 16k tERASE16KB = 250ms typ si 32bits
// block rewrite ram -> flash : 16us / word soit pour 4000 words : 64ms 

typedef enum { E_OK, E_NOSPC, E_NOTFOUND = -1} RES;

int nvrecord_used(); // number of records used. 
RES nvrecord_read(uint16_t id, uint16_t *value); // OK / NOTFOUND
RES nvrecord_write(uint16_t id, uint16_t value); // OK / NOSPC


/*defrag : 
 beware writing to flash data running from flash !
 - in second stage bootloader (et donc run from ram) 
 - in first stage bootloader 
 	- stalls... cela dit serialisé dc OK - sinon copy to stack !
*/
void nvrecord_defrag(void *tmp_data); 
// needs tempo data space of 4*MAX_RECORDS (typ 16k).
// needs a bit of time


//----------------------------------------------------

/* 
Small high scores lib  
storage of 16bit score + 16bit user
	(16bit = 3 5 bits letters)
*/
#define HISCORE_LEN 8 // 8 fits in one block 

// Loads internal table ; Id of first in record (should be a multiple of 16)
void init_high_score(int app_id); // 12 bits App_ID

// store if needed, update & keeps high score table sorted
void add_high_score(uint16_t user, uint16_t score);

// index=0 = highest score
// returns score << 16 | user 
// if returned id=0xffff / score= not found
uint32_t get_high_score(int index); // score <<16 | score (after initialization !) 



