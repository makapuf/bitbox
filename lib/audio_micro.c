// audio generatiuon through DAC 1
#include <stdint.h>
#include "stm32f4xx.h"
#include "bitbox.h"
#include "system.h" // interrupt handling

static uint16_t audio_buffer[BITBOX_SNDBUF_LEN]; // u8 mono samples

void audio_init()
{

}


// default empty implementation (silence)
__attribute__((weak)) void game_snd_buffer(uint16_t *buffer, int len) {}
