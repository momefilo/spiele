//momefilo Desing
#ifndef ranking_h
#define ranking_h 1

#define HIGHSCORECOUNT 5

void ranking_init(uint8_t flash_stage);
void paint_Highscore();
void paint_Menu();
void paint_Score(uint32_t score);
void paint_Rang(uint32_t rang);
uint8_t set_Score(uint32_t score);

#endif
