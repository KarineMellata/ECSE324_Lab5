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
	int break_code = 0;
	double time = 0;
	double frequency = 0.1;
	float volume = 1000;
	int write = 0;
	int write1 = 0;
	int_setup(1, (int[]){199});
	int i;
	HPS_TIM_config_t hps_tim_t;
	hps_tim_t.tim = TIM1;
	hps_tim_t.timeout = 480000;
	hps_tim_t.LD_en = 1;
	hps_tim_t.INT_en = 1;
	hps_tim_t.enable = 1;
	HPS_TIM_config_ASM(&hps_tim_t);
	/* This switch statement will check for what
	key has just been pressed, and will
	play the corresponding frequency.
	Order of keys : A,S,D,F,J,K,L,;
	*/
		if(hps_tim1_int_flag){ 
			hps_tim1_int_flag = 0;
			time += 0.48;
		for(i=0;i<8;i++){
			if(is_key_pressed[i]){
				printf("hello");
				write1 = output_w_volume(frequencies[i], time, volume);
				write += write1;
			}
		}
		if(!no_break_code){
			printf("there is a break code");
			write -= write1;
		}
		if(is_key_pressed[8]){
			volume = volume * 2;
		}
		if(is_key_pressed[9]){
			volume = volume / 2;
		}
}
}

void handle_keyboard(){
	char *key_pressed;
	double time = 0;
	double frequency = 0;
	float volume = 100;
	int_setup(1, (int[]){199});
	int output = 0;
	HPS_TIM_config_t hps_tim_t;
	hps_tim_t.tim = TIM0;
	hps_tim_t.timeout = 90000000;
	hps_tim_t.LD_en = 1;
	hps_tim_t.INT_en = 1;
	hps_tim_t.enable = 1;
	HPS_TIM_config_ASM(&hps_tim_t);
	if(hps_tim0_int_flag){
		hps_tim0_int_flag = 0;
		while(read_ps2_data_ASM(key_pressed)){
		switch(*key_pressed){
			case 0xF0:
				printf("breakcode\n");
				no_break_code = 0;
			case 0x1C:
				printf("key A\n");
				no_break_code = 1;
				is_key_pressed[0] = 1;
				break;
			case 0x1B:
				printf("key S\n");
				no_break_code = 1;
				is_key_pressed[1] = 1;
				break;
			case 0x23:
				printf("key D\n");
				no_break_code = 1;
				is_key_pressed[2] = 1;
				break;
			case 0x2B:
				no_break_code = 1;
				is_key_pressed[3] = 1;
				break;
			case 0x3B:
				no_break_code = 1;
				is_key_pressed[4] = 1;
				break;
			case 0x42:
				no_break_code = 1;
				is_key_pressed[5] = 1;
				break;
			case 0x4B:
				no_break_code = 1;
				is_key_pressed[6] = 1;
				break;
			case 0x4C:
				no_break_code = 1;
				is_key_pressed[7] = 1;
				break;
			case 0xE075:
				no_break_code = 1;
				is_key_pressed[8] = 1;
				break;
			case 0xE072:
				no_break_code = 1;
				is_key_pressed[9] = 1;
				break;
			default:
				no_break_code = 1;
				break;
			}
		}
	}
}


/*CONTROL WAVES*/
int main() {
	while(1){
		handle_keyboard();
		make_sound_w_interrupts();
	}
	return 0;
}

