
/*
EEprom in flash

16ko = 4096 slots (bien)
1 slot = ID 16b data 16b id = block ID
+ c'est bas (grand id) + c nouveau
*/

#include "eeprom.h"
#include "stm32f4xx.h"

#define FLASH_PAGE 0x08004000 
#define FLASH_SECTOR 1 

// flash record
typedef struct {
	uint16_t id, value; // id = 12 bits applicaiton ID, 4 bits sub_id (per-game)
} NVRecord;


NVRecord *flash_data = (NVRecord*)(FLASH_PAGE);


inline void flash_wait()
{
	while (FLASH->CR & FLASH_BSY);
}

void flash_optkey()
{
	// unlock optkey sequence 
	FLASH->OPTKEYR=0x08192A3B; // OPTKEY1
	FLASH->OPTKEYR=0x4C5D6E7F; // OPTKEY2
}

void flash_key()
{
	// unlock sequence 
	FLASH->KEYR=0x45670123; // KEY1
	FLASH->KEYR=0xCDEF89AB; // KEY2
}


void flash_lock()
{
	FLASH->CR|= FLASH_LOCK;
	FLASH->OPTCR|= FLASH_LOCK;
}


void flash_erase()
{
	flash_wait();
	flash_key();

	FLASH->CR |= FLASH_SER;
	FLASH->CR |= FLASH_SNB*FLASH_SECTOR;
	FLASH->CR |= FLASH_STRT;

	flash_lock();
	flash_wait();
}


void flash_rw()
{
	flash_wait();

	flash_key();
	// set width = 32bits
	FLASH->CR &= ~FLASH_PSIZE*3; // clear 2 bits
	FLASH->CR |= FLASH_PSIZE*2; // set 10 = x32
	FLASH->CR |= FLASH_STRT;

	flash_optkey();	
	FLASH->OPTCR |= nWRP*FLASH_SECTOR; // allow sector 2 for writing 
	FLASH->OPTCR |= OPTSTRT; // start 

	flash_lock();

	flash_wait();
}


void flash_ro()
{
	flash_wait();

	// prevent sector from writing  (clear nonwrite protect)
	flash_optkey();
	FLASH->OPTCR &= nWRP*FLASH_SECTOR; 
	FLASH->OPTCR |= OPTSTRT; // start 
	
	flash_lock();
	flash_wait();
}


int nvrecord_read(uint16_t id, uint16_t *value) // -1 if not found, OK if found
{
	// partir de la fin, remonter tt que ID pas trouve
	for (int i=MAX_RECORDS ;i>=0;i--)
	{
		if (flash_data[i].id==id) 
			*value= flash_data[i].value;
			return E_OK;
	}
	return E_NOTFOUND;
}

int nvrecord_used()
{
	int i=MAX_RECORDS;
	while (i>=0)
	{	
		if (flash_data[i].id != 0xffff) 
			return i;
		i--;
	}
	return 0; // all free
}

int nvrecord_write(uint16_t id, uint16_t value)
{
	// full ?
	if (flash_data[MAX_RECORDS-1].id != 0xffff) return E_NOSPC;

	// Set flash writable

	int i=MAX_RECORDS-2;
	while (i>=0 && flash_data[i+1] == 0xffff ) i--; 
	// case empty : i=-1 here
	
	flash_rw();
	flash_data[i+1] = (NVRecord){.id = id, .value=value}; // WRITE to flash
	flash_ro();

	return E_OK;
}


void nvrecord_defrag(NVRecord *tmp_data) 
/*
 * NVRecord : tmp space able to store MAX_RECORDS data
 */
{
	int dst_i=0; dst_nb=0;
	uint16_t tmp; // tmp record

	// defrag to ram
	for (int src_i=MAX_RECORDS;src_i>=0;src_i--)
	{
		tmp = flash_data[src_i];
		if ( tmp.id == 0xffff) continue; // skip empty 
		// test if this id is already known, if so skip
		for (dst_i=0;dst_i<dst_nb;dst_i++)
			if (tmp_data[dst_i].id==tmp.id) 
				break

		if (dst_i==dst_nb) // not found : add
			tmp_data[dst_nb++] = tmp;
	}

	// erase block
	flash_erase(FLASh_BLOCK);

	// write block
	flash_rw();
	for (dst_i=0;dst_i<dst_nb;dst_i++)
		flash_data[i]=tmp_data[dst_i];
	flash_ro();

}

// -- keeping high scores : an example.

static uint32_t high_scores[HISCORE_LEN]; 
// score <<16 | id so that score is stored with more importance (sorting).
// names not included here


void init_high_score(int app_id)
{
	uint16_t tmp;

	// read_list to memory & sort 
	for (int i=0;i<HISCORE_LEN;i++)
	{	
		tmp = 0; // default
		nvrecord_read(app_id<<4 | i, *tmp); // if not found, keep default
		high_scores[i]=tmp<<16 | app_id<<4 | i; 
	}
	sort_list();
}

void add_high_score(uint16_t user,score) 
{
	if (score > high_scores[8]>>16)
	{
		uint16_t old_id = (high_scores[8] &0xffff);
		// replace last one with new
		high_scores[8]=score <<16 | old_id;
		// ensure at least 2 records free ?
		nvrecord_write(old_id, score);
		nvrecord_write(old_id+HISCORE_LEN, user);
	}
}


uint32_t get_high_score(int index)
{
	if (n<0 || n>MAX_RECORDS-1) return E_NOTFOUND;
	uint16_t usr;
	nvrecord_read((high_scores[index]&0xffff)+HISCORE_LEN,&usr);
	return (high_scores[index] & 0xffff0000) | usr;
}


static void sort_list() 
// simple insertion sort
{
	int i, j, tmp;
	for (i = 1; i < HISCORE_LEN; i++) {
        j = i;
        while (j > 0 && high_scores[j - 1] > high_scores[j]) // trie pas les ID
        {
			tmp = high_scores[j];
			high_scores[j] = high_scores[j - 1];
			high_scores[j - 1] = tmp;
			j--;
        }
  }
}

/*
run from ram : 
https://my.st.com/public/STe2ecommunities/mcu/Lists/cortex_mx_stm32/Flat.aspx?RootFolder=/public/STe2ecommunities/mcu/Lists/cortex_mx_stm32/Running%20a%20particular%20function%20from%20RAM&FolderCTID=0x01200200770978C69A1141439FE559EB459D7580009C4E14902C3CDE46A77F0FFD06506F5B&currentviews=238


encode to flash => 0b1111111111111111 peut etre ecrit 16 fois en decrementant sans erase (monotone) . quelles données ?  

*/


/* 

test : 
	ecrire N valeurs depuis M sources (certaines se suivent d'autres non)
	lire une valeur trouvee (de chaque src)
	lire une valeur non trouvee
	remplir 
	defrag

	test hi scores.
	- table semi remplie
	- teble pleine, add new add non 

	system_wide : language, current ROM, bootloader id, registered name, play time ?

*/