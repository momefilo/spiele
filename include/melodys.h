#ifndef momefilo_melodys_h
#define momefilo_melodys_h 1

#include "pico/stdlib.h"

#define SOUND_YEH 1
#define SOUND_BAH 2
#define SOUND_ROW 3
#define SOUND_FALL 4
#define SOUND_YUP 5

void melodys_play(uint8_t melody);
void melodys_init();
#endif
