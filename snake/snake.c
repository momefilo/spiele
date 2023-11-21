// momefilo Desing
#include "../include/libs/ili9341/ili9341.h"
#include "../include/ranking.h"
#include "../include/buttons.h"
#include "../include/melodys.h"
#include "graphics/snake15x15.h"
#include "snake.h"
#include "hardware/adc.h"
#include <stdlib.h>

#define SNAKE_Left	0
#define SNAKE_Right	1
#define SNAKE_Up	2
#define SNAKE_Down	3
#define EMPTY		0
#define HEAD		1
#define TAIL		2
#define FOOD		3
struct POS{
	uint16_t x;
	uint16_t y;
} *Tail, Act_pos, New_pos;
uint8_t Snake_direction, Snake_speed;
uint16_t SnakeArea[16][20];
uint16_t Tail_cnt;
uint32_t Snake_Score;
int Snake_Salz;
uint8_t Snake_DigitsOffset = 20;

uint8_t snake_getSpeed(){ return Snake_speed; }
void snake_input(){
	if(gpio_get(BUTTON_U)){Snake_direction = SNAKE_Up;}
	if(gpio_get(BUTTON_D)){Snake_direction = SNAKE_Down;}
	if(gpio_get(BUTTON_L)){Snake_direction = SNAKE_Left;}
	if(gpio_get(BUTTON_R)){Snake_direction = SNAKE_Right;}
}
void snake_set_figure(uint8_t fig, struct POS mypos){
	SnakeArea[mypos.x][mypos.y] = fig;
	uint16_t area[] = {
		mypos.x * 15,
		mypos.y * 15 + Snake_DigitsOffset,
		mypos.x * 15 + 14,
		mypos.y * 15 + Snake_DigitsOffset + 14};
	drawRect(area, SNAKE[fig]);
}

void snake_set_food(){
	int r = rand_r(&Snake_Salz);
	struct POS mypos;
	mypos.x = r % 16;
	mypos.y = r % 20;
	while(SnakeArea[mypos.x][mypos.y] > 0){
		r = rand_r(&Snake_Salz);
		mypos.x = r % 16;
		mypos.y = r % 20;
	}
	snake_set_figure(FOOD, mypos);
	Snake_Salz++;
	Snake_Score ++;
	paint_Score(Snake_Score);
	if(Tail_cnt % 3 == 0){
		if(Snake_speed>2){ Snake_speed = Snake_speed - 2; }
	}
}
void snake_init(uint8_t progId){
	buttons_init();
	ili9341_init();
	setOrientation(VERTICAL);
	melodys_init();
	ranking_init(progId);
	adc_init();
	Tail = (struct POS*)malloc(sizeof(struct POS));
	adc_select_input(0);
	Snake_Salz = adc_read()*2;
	sleep_us(1);
	adc_select_input(1);
	Snake_Salz = Snake_Salz + adc_read()+1;
}
void snake_reset(){
	for(int x=0; x<16; x++){
		for(int y=0; y<20; y++){
			SnakeArea[x][y] = 0;
			uint16_t area[] = {
				x * 15,
				y * 15 + Snake_DigitsOffset,
				x * 15 + 14,
				y * 15 + Snake_DigitsOffset + 14};
			drawRect(area, SNAKE[EMPTY]);
		}
	}
	Snake_speed = 199;
	Tail_cnt = 0;
	Tail = realloc(Tail, 0);
	Act_pos.x = 8;
	Act_pos.y = 10;
	snake_set_figure(HEAD, Act_pos);
	Snake_direction = SNAKE_Up;
	snake_set_food();
	Snake_Score = 0;
	paint_Score(Snake_Score);
}
bool snake_end(){
	bool ret;
	uint8_t ranking = set_Score(Snake_Score);
	paint_Highscore();
	if(ranking > 0){
		paint_Rang(ranking);
		if(ranking < 2)melodys_play(SOUND_YEH);
		else melodys_play(SOUND_YUP);
	}
	else{
		paint_Score(Snake_Score);
		melodys_play(SOUND_BAH);
	}
	while( (! gpio_get(BUTTON_R)) && (! gpio_get(BUTTON_L) && \
		(! gpio_get(BUTTON_U)) && (! gpio_get(BUTTON_D))) ){}
	paint_Menu();
	sleep_ms(500);
	bool auswahl = false;
	while(! auswahl){
		if(gpio_get(BUTTON_U)){ ret = true; auswahl = true; }
		if(gpio_get(BUTTON_D)){ ret = false; auswahl = true; }
	}
	snake_reset();
	return ret;
}
bool snake_move(){
	bool has_eat = false;
	//get new position
	if(Snake_direction == SNAKE_Up){
		if(Act_pos.y > 0) {New_pos.y = Act_pos.y - 1;}
		else {New_pos.y = 19;}
		New_pos.x = Act_pos.x;
	}
	if(Snake_direction == SNAKE_Down){
		if(Act_pos.y < 19) {New_pos.y = Act_pos.y + 1;}
		else {New_pos.y = 0;}
		New_pos.x = Act_pos.x;
	}
	if(Snake_direction == SNAKE_Left){
		if(Act_pos.x > 0) {New_pos.x = Act_pos.x - 1;}
		else {New_pos.x = 15;}
		New_pos.y = Act_pos.y;
	}
	if(Snake_direction == SNAKE_Right){
		if(Act_pos.x < 15) {New_pos.x = Act_pos.x + 1;}
		else {New_pos.x = 0;}
		New_pos.y = Act_pos.y;
	}

	//check new position
	//food?
	if(SnakeArea[New_pos.x][New_pos.y] == FOOD) has_eat = true;
	//tail?
	if(SnakeArea[New_pos.x][New_pos.y] == TAIL) { return snake_end(); }
	//move to new position
	if(Tail_cnt>0) {snake_set_figure(TAIL, Act_pos);}
	else {snake_set_figure(EMPTY, Act_pos);}
	snake_set_figure(HEAD, New_pos);

	//set end of tail
	if(has_eat){
		melodys_play(SOUND_FALL);
		Tail = realloc(Tail, (Tail_cnt + 1) * sizeof(struct POS));
		Tail[Tail_cnt] = Act_pos;
		Tail_cnt++;
		snake_set_food();
		paint_Score(Snake_Score);
	}
	else{
		if(Tail_cnt > 0){
			snake_set_figure(EMPTY, Tail[0]);
			for(int i=1; i<Tail_cnt; i++){
				Tail[i-1] = Tail[i];
			}
			Tail[Tail_cnt-1] = Act_pos;
		}
	}
	Act_pos = New_pos;
	return true;
}
