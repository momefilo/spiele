//momefilo Desing
#ifndef snake_h
#define snake_h 1

#include "pico/stdlib.h"

void snake_init(uint8_t progId);
void snake_input();
bool snake_move();
void snake_reset();
uint8_t snake_getSpeed();


#endif
