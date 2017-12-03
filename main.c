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
	int time = 0;
	while(1){
		write = output(frequency, time);
		audio_write_data_ASM(write, write);
		time = time + 1;
		if(time > 48000){
			time = 0;
			}
	}
}
/*
void make_sound_w_interrupts(double frequency, float volume, double time){
	audio_write_data_ASM(write, write);
}
*/

/* Part 1 MAKE WAVES
int main() {	
	make_sound(100);
	return 0;
}

*/

/*CONTROL WAVES*/
int main() {
	int write = 0;
	char *key_pressed;
	int x = 0;
	int break_code = 0;
	int time = 0;
	double frequency = 0.1;
	float volume = 1000;
	int_setup(1, (int[]){199});
	int output = 0;

	HPS_TIM_config_t hps_tim_t;
	hps_tim_t.tim = TIM0;
	hps_tim_t.timeout = 100;
	hps_tim_t.LD_en = 1;
	hps_tim_t.INT_en = 1;
	hps_tim_t.enable = 1;
	HPS_TIM_config_ASM(&hps_tim_t);

	do{
		/* This switch statement will check for what
		   key has just been pressed, and will
		   play the corresponding frequency.
		   Order of keys : A,S,D,F,J,K,L,;
		*/
		if(hps_tim0_int_flag){ 
			hps_tim0_int_flag = 0;
			time += 0.0001f;
		}
		while(read_ps2_data_ASM(key_pressed)){
		switch(*key_pressed){
			case 0xF0:
				frequency = 0;
				break;
			case 0x1C:
				output += output_w_volume(130.813, time, volume);
				break;
			case 0x1B:
				output += output_w_volume(146.832, time, volume);
				break;
			case 0x23:
				output += output_w_volume(164.814, time, volume);
				break;
			case 0x2B:
				output += output_w_volume(174.614, time, volume);
				break;
			case 0x3B:
				output += output_w_volume(195.998, time, volume);
				break;
			case 0x42:
				output += output_w_volume(220.000, time, volume);
				break;
			case 0x4B:
				output += output_w_volume(246.942, time, volume);
				break;
			case 0x4C:
				output += output_w_volume(261.626, time, volume);
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
		audio_write_data_ASM(output, output);
	} while(1);
	return 0;
}

