#include "./drivers/inc/vga.h"
#include "./drivers/inc/ISRs.h"
#include "./drivers/inc/LEDs.h"
#include "./drivers/inc/audio.h"
#include "./drivers/inc/HPS_TIM.h"
#include "./drivers/inc/int_setup.h"
#include "./drivers/inc/wavetable.h"
#include "./drivers/inc/pushbuttons.h"
#include "./drivers/inc/ps2_keyboard.h"
#include "./drivers/inc/HEX_displays.h"
#include "./drivers/inc/slider_switches.h"

int output(double frequency, int time){
		float amplitude= 1;
		float sound;
		float decimal = frequency - (int)frequency;
		int index = ((int)frequency *time)% 48000;
		if(decimal == 0){ sound = amplitude * sine[index]; }
		else { sound = amplitude * ((1-decimal)*sine[index] + decimal*sine[index+1]); }
		return (int)sound;
}

int output_w_volume(double frequency, int time, float amplitude){
		float sound;
		float decimal = frequency - (int)frequency;
		int index = ((int)frequency *time)% 48000;
		if(decimal == 0){ sound = amplitude * sine[index]; }
		else { sound = amplitude * ((1-decimal)*sine[index] + decimal*sine[index+1]); }
		return (int)sound;
}


void make_sound(double frequency){
	int x = 0;
	int write = 0;
	HPS_TIM_config_t hps_tim;
	hps_tim.tim = TIM0;
	hps_tim.timeout = 1000000;
	hps_tim.LD_en = 1;
	hps_tim.INT_en = 0;
	hps_tim.enable = 1;

	HPS_TIM_config_ASM(&hps_tim); //Config timer
		
	while (x<=48000) {
		write = output(frequency, x);
		if (audio_write_data_ASM(write, write)) 
			 x = x+1; 
		if( x > 48000) 
			 x = 0;
	}
}

void make_sound_w_interrupts(double frequency, float volume){
	int x = 0;
	int write = 0;
	char *key_pressed1;
	int break_code = 0;

	while (x<=48000 && break_code == 0) {
		read_ps2_data_ASM(key_pressed1);
		write = output_w_volume(frequency, x, volume);
		if(*key_pressed1 == 0xF0){
			break_code = 1;}
		printf("%d\n", write);
		audio_write_data_ASM(write, write);
		if (audio_write_data_ASM(write, write)){
			 x = x+1;}
		if(x > 48000){
			 x = 0;}
}
}


/* MAKE WAVES*/ /* 
int main() {		
	while (1) {
		make_sound(100);
}
	return 0;
}


*/

/*CONTROL WAVES*/
int main() {
	int write = 0;
	char *key_pressed;
	int x = 0;
	int break_code = 0;
	double frequency = 0.1;
	float volume = 1;
	do{
		/* This switch statement will check for what
		   key has just been pressed, and will
		   play the corresponding frequency.
		   Order of keys : A,S,D,F,J,K,L,;
		*/
		if(read_ps2_data_ASM(key_pressed)){
		switch(*key_pressed){
			case 0xF0:
				frequency = 0;
				break;
			case 0x1C:
				frequency = 130.813;
				break;
			case 0x1B:
				frequency = 146.832;
				break;
			case 0x23:
				frequency = 164.814;
				break;
			case 0x2B:
				frequency = 174.614;
				break;
			case 0x3B:
				frequency = 195.998;
				break;
			case 0x42:
				frequency = 220.000;
				break;
			case 0x4B:
				frequency = 246.942;
				break;
			case 0x4C:
				frequency = 261.626;
				break;
			case 0xE075:
				frequency = 0;
				volume = volume + 2;
				break;
			case 0xE072:
				frequency = 0;
				volume = volume - 2;
				break;
			default:
				frequency = 0;
				break;
			}
		}
		if(frequency != 0.1){
			make_sound_w_interrupts(frequency, volume);
		}
	} while(1);
	return 0;
}
