// momefilo Desing
#include <stdio.h>
#include "pico/stdlib.h"
#include "include/buttons.h"
#include "include/libs/ili9341/ili9341.h"
//#include "include/libs/flash/flash.h"
#include "tetris/tetris.h"
#include "snake/snake.h"
#include "klotski/klotski.h"

void paintEntry(){
	uint16_t yend = 299;
	uint16_t pos[] = {0, 0, 24, yend};
	paintRectGradient(pos, 0x0000, 0x001F);
	pos[0] = 25; pos[2] = 214;
	paintRect(pos, 0x841F);
	pos[0] = 215; pos[2] = 239;
	paintRectGradient(pos, 0x001F, 0x0000);
	setBgColor(0x841F);
	setFgColor(0x0010);
	uint16_t tpos[] = {56,40};
	writeText16x16(tpos, "momefilo", 8, false, false);
	tpos[0] = 90; tpos[1] = 60;
	writeText12x12(tpos, "games", 5, false, false);
	pos[0] = 25; pos [1] = 0, pos[2] = 215; pos [3] = 5,
	paintRect(pos, 0x001B);
	pos[0] = 25; pos [1] = 294, pos[2] = 215; pos [3] = yend,
	paintRect(pos, 0x001B);
	pos[0] = 39; pos[1] = 81; pos[2] = 65; pos[3] = 81;
	paintRectGradient(pos, 0xF800, 0xFFE0);
	pos[0] = 65; pos[2] = 94;
	paintRectGradient(pos, 0xFFE0, 0x07E0);
	pos[0] = 94; pos[2] = 121;
	paintRectGradient(pos, 0x07E0, 0x07FF);
	pos[0] = 121; pos[2] = 148;
	paintRectGradient(pos, 0x07FF, 0x001F);
	pos[0] = 148; pos[2] = 175;
	paintRectGradient(pos, 0x001F, 0xF81F);
	pos[0] = 175; pos[2] = 202;
	paintRectGradient(pos, 0xF81F, 0xF800);
	pos[0] = 27; pos [1] = 7, pos[2] = 213; pos [3] = 7;
	paintRect(pos, 0xFFE0);
	pos[0] = 213; pos [1] = 7, pos[2] = 213; pos [3] = 292;
	paintRect(pos, 0xFFE0);
	pos[0] = 27; pos [1] = 292, pos[2] = 213; pos [3] = 292;
	paintRect(pos, 0xFFE0);
	pos[0] = 27; pos [1] = 7, pos[2] = 27; pos [3] = 292;
	paintRect(pos, 0xFFE0);
	tpos[0] = 72; tpos[1] = 180;
	setFgColor(0x07E0);
	writeText16x16(tpos, "Tetris", 6, false, false);
	tpos[0] = 80; tpos[1] = 220;
	setFgColor(0xFFE0);
	writeText16x16(tpos, "Snake", 5, false, false);
}

int main(){
	stdio_init_all();
	sleep_ms(1);
	bool showEntry = true;
	bool Menu_pressed = false;
	ili9341_init();
	setOrientation(VERTICAL);
	buttons_init();
	while(1){
		if(showEntry){
			paintEntry();
			showEntry = false;
		}
		if(gpio_get(BUTTON_U) || gpio_get(BUTTON_D) || gpio_get(BUTTON_R)){
			if(Menu_pressed){
				showEntry = true;
				if(gpio_get(BUTTON_U)){
					Menu_pressed = false;
					tetris_init(0);
					while(true){
						uint16_t speed = get_Fallspeed();
						uint8_t loop = speed/100;
						for(int i=loop; i<speed; i=i+loop){
							if(get_Direction() > 1) break;
							sleep_us(loop*1000);
						}
						if(! fall_ActFigur()){
							if(! end_fall(true)){
								break;
							}
						}
						reset_Movecounter();
					}
					sleep_ms(500);
				}
				else if(gpio_get(BUTTON_D) && Menu_pressed){
					Menu_pressed = false;
					snake_init(1);
					snake_reset();
					while(true){
						snake_input();
						if(! snake_move()) break;
						snake_input();
						sleep_ms(snake_getSpeed());
						snake_input();

					}
					sleep_ms(500);
				}
				else if(gpio_get(BUTTON_R) && Menu_pressed){
					Menu_pressed = false;
					klotski_init(2);
					sleep_ms(500);
				}
			}
			else{ Menu_pressed = true; sleep_ms(100);}
		}
	}
}
