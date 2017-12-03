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
int is_key_pressed[10] = {0}; //Initializing all keys to not pressed
//Order is A,S,D,F,J,K,L,;,VOLUME_UP,VOLUME_DOWN
int frequencies[10] = {130.813, 146.832, 164.814, 174.614, 195.998, 220.000, 246.942, 261.626, 0, 0}; //All frequencies
int no_break_code = 1;
int time = 0;
int write = 0;

int output_w_volume(double frequency, int time, float amplitude){
		float sound;
		float decimal = frequency - (int)frequency;
		int index = ((int)frequency *time)% 48000;
		if(decimal == 0){ sound = amplitude * sine[index]; }
		else { sound = amplitude * ((1-decimal)*sine[index] + decimal*sine[index+1]); }
		return (int)sound;
}

void make_sound_w_interrupts(){
	char *key_pressed;
	float volume = 1000;
	int i;
	/* This switch statement will check for what
	key has just been pressed, and will
	play the corresponding frequency.
	Order of keys : A,S,D,F,J,K,L,;
	*/
	if(hps_tim1_int_flag){
		hps_tim1_int_flag = 0;
		time += 1;
		if(is_key_pressed[8]){
			volume = volume * 2;
		}
		if(is_key_pressed[9]){
			volume = volume / 2;
		}
		for(i=0;i<8;i++){
			if(is_key_pressed[i]){
				write = output_w_volume(frequencies[i], time, volume);
				write += write;
			}
		}
		audio_write_data_ASM(write, write);
		if(time > 48000){
			time = 0;
		}
	}
}

void handle_keyboard(){
	char *key_pressed;
	if(hps_tim0_int_flag){
		printf("yo");
		hps_tim0_int_flag = 0;
	if(read_ps2_data_ASM(key_pressed)){
		switch(*key_pressed){
			case 0xF0:
				no_break_code = 0;
			case 0x1C:
				is_key_pressed[0] = no_break_code;
				no_break_code = 1;
				break;
			case 0x1B:
				is_key_pressed[1] = no_break_code;
				no_break_code = 1;
				break;
			case 0x23:
				is_key_pressed[2] = no_break_code;
				no_break_code = 1;
				break;
			case 0x2B:
				is_key_pressed[3] = no_break_code;
				no_break_code = 1;
				break;
			case 0x3B:
				is_key_pressed[4] = no_break_code;
				no_break_code = 1;
				break;
			case 0x42:
				is_key_pressed[5] = no_break_code;
				no_break_code = 1;
				break;
			case 0x4B:
				is_key_pressed[6] = no_break_code;
				no_break_code = 1;
				break;
			case 0x4C:
				is_key_pressed[7] = no_break_code;
				no_break_code = 1;
				break;
			case 0xE075:
				is_key_pressed[8] = no_break_code;
				no_break_code = 1;
				break;
			case 0xE072:
				is_key_pressed[9] = no_break_code;
				no_break_code = 1;
				break;
			default:
				no_break_code = 1;
				break;
			}
		}
	}
}

void setup_timers(){
	int_setup(2, (int[]){200,199});

	//AUDIO TIMER
	HPS_TIM_config_t hps_tim_1;
	hps_tim_1.tim = TIM1;
	hps_tim_1.timeout = 20;
	hps_tim_1.LD_en = 1;
	hps_tim_1.INT_en = 1;
	hps_tim_1.enable = 1;
	HPS_TIM_config_ASM(&hps_tim_1);

	//KEYBOARD TIMER
	HPS_TIM_config_t hps_tim_2;
	hps_tim_2.tim = TIM0;
	hps_tim_2.timeout = 80000;
	hps_tim_2.LD_en = 1;
	hps_tim_2.INT_en = 1;
	hps_tim_2.enable = 1;
	HPS_TIM_config_ASM(&hps_tim_2);
	}


/*CONTROL WAVES*/
int main() {
	setup_timers();
	while(1){
		handle_keyboard();
		make_sound_w_interrupts();
	}
	return 0;
}

