#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "sound.h"

const uint16_t FreqArray[]={1047,1109,1175,1245,1319,1397,1480,1568,1661,1761,1865,1976};
uint Slice_num;
uint16_t Dauer = 1500;

void sound_note(uint8_t note, uint8_t dauer, uint8_t oktave){
	pwm_set_enabled(Slice_num, true);
	if(oktave == 1){ pwm_set_clkdiv(Slice_num, 80.f); }
	else{ pwm_set_clkdiv(Slice_num, 160.f); }
	pwm_set_wrap( Slice_num, (3125*1000)/FreqArray[note]);
	pwm_set_chan_level(Slice_num, PWM_CHAN_B, (3125*1000)/(FreqArray[note]*2));
	sleep_ms(Dauer/dauer);
	//pwm_set_clkdiv(Slice_num, 80.f);
	pwm_set_enabled(Slice_num, false);
}
void sound_Row(){
	uint8_t oktave = 0;
	uint8_t dauer = 64;
	for(uint8_t i=0; i<12; i=i+2)sound_note(i,dauer,oktave);
}
void sound_Fall(){
	uint8_t oktave = 0;
	uint8_t dauer = 16;
//	for(uint8_t i=11; i>=4; i=i-4)sound_note(i,dauer,oktave);
	sound_note(2,dauer,oktave);
}
void sound_Bah(){
	uint8_t oktave = 0;
	uint8_t dauer = 8;
	sound_note(5,dauer-2,oktave);
	sound_note(2,dauer-2,oktave);
	sound_note(4,dauer-2,oktave);
	sound_note(0,dauer-4,oktave);
}
void sound_Yup(){
	uint8_t oktave = 0;
	uint8_t dauer = 8;
	sound_note(0,dauer-2,oktave);
	sound_note(5,dauer-2,oktave);
	sound_note(2,dauer-2,oktave);
	sound_note(7,dauer-4,oktave);
}
void sound_Yeh(){
	uint8_t oktave = 0;
	uint8_t dauer = 8;
	sound_note(5,dauer,oktave);
	sound_note(9,dauer,oktave);
	sound_note(0,dauer-2,oktave+1);
	sound_note(7,dauer,oktave);
	sound_note(11,dauer,oktave);
	sound_note(2,dauer-2,oktave+1);
	sound_note(0,dauer,oktave+1);
	sound_note(4,dauer,oktave+1);
	sound_note(7,dauer-4,oktave+1);

}
void sound_PeerGynt(){
	uint8_t oktave = 1;
	sound_note(7,4,oktave);
	sound_note(4,4,oktave);
	sound_note(2,4,oktave);
	sound_note(0,4,oktave);
	sound_note(2,4,oktave);
	sound_note(4,4,oktave);
	sound_note(7,4,oktave);
	sound_note(4,4,oktave);
	sound_note(2,4,oktave);
	sound_note(0,4,oktave);
	sound_note(2,8,oktave);
	sound_note(4,8,oktave);
	sound_note(2,8,oktave);
	sound_note(4,8,oktave);
	sound_note(7,4,oktave);
	sound_note(4,4,oktave);
	sound_note(7,4,oktave);
	sound_note(9,4,oktave);
	sound_note(4,4,oktave);
	sound_note(9,4,oktave);
	sound_note(7,4,oktave);
	sound_note(4,4,oktave);
	sound_note(2,4,oktave);
	sound_note(0,1,oktave);
}

void Sound_play(uint8_t sound){
	if(sound == SOUND_BAH) sound_Bah();
	if(sound == SOUND_YEH) sound_Yeh();
	if(sound == SOUND_ROW) sound_Row();
	if(sound == SOUND_FALL) sound_Fall();
	if(sound == SOUND_YUP) sound_Yup();
}

void sound_init(){
	gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
	Slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
	pwm_set_clkdiv(Slice_num, 80.f);
}
