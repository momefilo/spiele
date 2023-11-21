#include "../libs/sound/sound.h"
#include "../melodys.h"
#include "pico/stdlib.h"

void melodys_init(){sound_init();}
void melody_Row(){
	uint8_t oktave = 0;
	uint8_t dauer = 64;
	for(uint8_t i=0; i<12; i=i+2)sound_play(i,dauer,oktave);
}
void melody_Fall(){
	uint8_t oktave = 0;
	uint8_t dauer = 16;
//	for(uint8_t i=11; i>=4; i=i-4)sound_play(i,dauer,oktave);
	sound_play(2,dauer,oktave);
}
void melody_Bah(){
	uint8_t oktave = 0;
	uint8_t dauer = 8;
	sound_play(5,dauer-2,oktave);
	sound_play(2,dauer-2,oktave);
	sound_play(4,dauer-2,oktave);
	sound_play(0,dauer-4,oktave);
}
void melody_Yup(){
	uint8_t oktave = 0;
	uint8_t dauer = 8;
	sound_play(0,dauer-2,oktave);
	sound_play(5,dauer-2,oktave);
	sound_play(2,dauer-2,oktave);
	sound_play(7,dauer-4,oktave);
}
void melody_Yeh(){
	uint8_t oktave = 0;
	uint8_t dauer = 8;
	sound_play(5,dauer,oktave);
	sound_play(9,dauer,oktave);
	sound_play(0,dauer-2,oktave+1);
	sound_play(7,dauer,oktave);
	sound_play(11,dauer,oktave);
	sound_play(2,dauer-2,oktave+1);
	sound_play(0,dauer,oktave+1);
	sound_play(4,dauer,oktave+1);
	sound_play(7,dauer-4,oktave+1);

}
void melody_PeerGynt(){
	uint8_t oktave = 1;
	sound_play(7,4,oktave);
	sound_play(4,4,oktave);
	sound_play(2,4,oktave);
	sound_play(0,4,oktave);
	sound_play(2,4,oktave);
	sound_play(4,4,oktave);
	sound_play(7,4,oktave);
	sound_play(4,4,oktave);
	sound_play(2,4,oktave);
	sound_play(0,4,oktave);
	sound_play(2,8,oktave);
	sound_play(4,8,oktave);
	sound_play(2,8,oktave);
	sound_play(4,8,oktave);
	sound_play(7,4,oktave);
	sound_play(4,4,oktave);
	sound_play(7,4,oktave);
	sound_play(9,4,oktave);
	sound_play(4,4,oktave);
	sound_play(9,4,oktave);
	sound_play(7,4,oktave);
	sound_play(4,4,oktave);
	sound_play(2,4,oktave);
	sound_play(0,1,oktave);
}

void melodys_play(uint8_t melody){
	if(melody == SOUND_BAH) melody_Bah();
	if(melody == SOUND_YEH) melody_Yeh();
	if(melody == SOUND_ROW) melody_Row();
	if(melody == SOUND_FALL) melody_Fall();
	if(melody == SOUND_YUP) melody_Yup();
}
