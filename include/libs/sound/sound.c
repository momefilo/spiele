#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "sound.h"

const uint16_t FreqArray[]={1047,1109,1175,1245,1319,1397,1480,1568,1661,1761,1865,1976};
uint Slice_num;
uint16_t Dauer = 1500;


void sound_play(uint8_t note, uint8_t dauer, uint8_t oktave){
	pwm_set_enabled(Slice_num, true);
	if(oktave > 1){ pwm_set_clkdiv(Slice_num, 40.f); }
	else if(oktave == 1){ pwm_set_clkdiv(Slice_num, 80.f); }
	else{ pwm_set_clkdiv(Slice_num, 160.f); }
	pwm_set_wrap( Slice_num, (3125*1000)/FreqArray[note]);
	pwm_set_chan_level(Slice_num, PWM_CHAN_B, (3125*1000)/(FreqArray[note]*2));
	sleep_ms(Dauer/dauer);
	//pwm_set_clkdiv(Slice_num, 80.f);
	pwm_set_enabled(Slice_num, false);
}

void sound_PeerGynt(){
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

void sound_init(){
	gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
	Slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
	pwm_set_clkdiv(Slice_num, 80.f);
}
