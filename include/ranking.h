// momefilo Desing
#ifndef momefilo_ranking_h
#define momefilo_ranking_h 1

#include "pico/stdlib.h"

#define HIGHSCORECOUNT 5

void ranking_init(uint8_t flash_stage);
void paint_Highscore();
void paint_Menu();
void paint_Score(uint32_t score);
void paint_Rang(uint32_t rang);
uint8_t set_Score(uint32_t score);
int get_Salz();

#endif
