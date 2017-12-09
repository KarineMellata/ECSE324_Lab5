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
double time = 0;
int global_write = 0;
float volume = 10; 
//Colors array
int colors[6] = {0xFFFF00, 0xFF6347, 0xDC143C, 0x3CB371, 0x9400D3, 0x00FF00};

/*GENERATE THE SIGNAL*/
int output_w_volume(double frequency, int time, float amplitude){
	float sound;
	float decimal = frequency - (int)frequency;
	int index = ((int)frequency *time)% 48000; //This logic has been taken from the lab handout
	if(decimal == 0){ sound = amplitude * sine[index]; } //Taking scale from wavetable
	else { sound = amplitude * ((1-decimal)*sine[index] + decimal*sine[index+1]); }
	return (int)sound; //Returning sample
}

/*DRAW THE WAVES*/
void display_waves(){
	if(hps_tim2_int_flag){ //This timeout goes at the rate of the display of the VGA
		//Time goes from 0 to 48000, but it can't go over 319 cause of the screen width, hence the division by 151
		//global_write goes from 0 to 9000000 (Im actually not entirely sure, to be verified) hence the division by 35500
		//120 is added to have a centered sine wave
		//Colors are changing for fun :)
		//35500, 151, volume*35500
		if(time <= 8000){
		VGA_draw_point_ASM(time/200, (global_write/(10000000)) + 120 , colors[0]); //Time is global, hence no need for a loop here
		}
		else if(time > 8000 && time <= 16000){
		VGA_draw_point_ASM(time/200, (global_write/(10000000)) + 120 , colors[1]); //Time is global, hence no need for a loop here
		}
		else if(time > 16000 && time <= 24000){
		VGA_draw_point_ASM(time/200, (global_write/(10000000)) + 120 , colors[2]); //Time is global, hence no need for a loop here
		}
		else if(time > 24000 && time <= 32000){
		VGA_draw_point_ASM(time/200, (global_write/(10000000)) + 120 , colors[3]); //Time is global, hence no need for a loop here
		}
		else if(time > 32000 && time <= 40000){
		VGA_draw_point_ASM(time/200, (global_write/(10000000)) + 120 , colors[4]); //Time is global, hence no need for a loop here
		}
		else if(time > 40000 && time <= 48000){
		VGA_draw_point_ASM(time/200, (global_write/(10000000)) + 120 , colors[5]); //Time is global, hence no need for a loop here
		}
		hps_tim2_int_flag = 0; //Setting it back to 0
	}
} 

/*MAKE THE SOUND*/
void make_sound_w_interrupts(){
	int i;
	int write = 0;
	int write1 = 0;
	/* This switch statement will check for what
	key has just been pressed, and will
	play the corresponding frequency.
	Order of keys : A,S,D,F,J,K,L,;
	*/
	if(hps_tim1_int_flag && no_break_code){ //Running this a timer's pace, and only if we are not in break mode state
		hps_tim1_int_flag = 0;
		if(is_key_pressed[8]){
			volume = volume + 1;
		}
		if(is_key_pressed[9]){
			volume = volume - 1;
		}
		for(i=0;i<8;i++){ //Check for all keys currently pressed
			if(is_key_pressed[i]){
				write1 = output_w_volume(frequencies[i], time, volume); //Generate signal
				write += write1; //Add signals together if needed
			}
	}
		global_write = write;
		if(audio_write_data_ASM(write, write)){ //Write to audio FIFO
			time += 1;
		}

		if(time == 48000){ //Restart time after 1 period has passed
			time = 0;
			VGA_clear_pixelbuff_ASM(); //Make sure to clear all buffers first
			VGA_clear_charbuff_ASM();
		}
}
}

/*HANDLE WHEN KEY ARE PRESSED*/
void handle_keyboard(){
	char *key_pressed;
	if(hps_tim0_int_flag){
	while(read_ps2_data_ASM(key_pressed)){ //If a key has been pressed 
		switch(*key_pressed){
			case 0xF0: //This is the break code. It will mean we entered break code mode
				no_break_code = 0;
				break;
			case 0x1C: //A
				is_key_pressed[0] = no_break_code; //If in break code stage, disable the key, if not enable the key
				no_break_code = 1; //Exit break code mode if we were in it
				break;
			case 0x1B: //S
				is_key_pressed[1] = no_break_code;
				no_break_code = 1;
				break;
			case 0x23: //D
				is_key_pressed[2] = no_break_code;
				no_break_code = 1;
				break;
			case 0x2B: //F
				is_key_pressed[3] = no_break_code;
				no_break_code = 1;
				break;
			case 0x3B: //J
				is_key_pressed[4] = no_break_code;
				no_break_code = 1;
				break;
			case 0x42: //K
				is_key_pressed[5] = no_break_code;
				no_break_code = 1;
				break;
			case 0x4B: //L
				is_key_pressed[6] = no_break_code;
				no_break_code = 1;
				break;
			case 0x4C: //;
				is_key_pressed[7] = no_break_code;
				no_break_code = 1;
				break;
			case 0x75: //Up key (keypad)
				is_key_pressed[8] = no_break_code;
				no_break_code = 1;
			case 0x72: //Down key (keypad)
				is_key_pressed[9] = no_break_code;
				no_break_code = 1;
			default: //If any other key has been pressed this statement makes sure that the break code isn't taken into account
				no_break_code = 1;
				break;
			}
		}
}
	hps_tim0_int_flag = 0;
	make_sound_w_interrupts(); //We are now ready to make the sound
}

void setup_timers(){
	int_setup(3, (int[]){199,200,201}); //Setting up three timers for the three different speed of I/O units

	//KEYBOARD TIMER
	HPS_TIM_config_t hps_tim_0;
	hps_tim_0.tim = TIM0;
	hps_tim_0.timeout = 80000; //10.9 characters per second 
	hps_tim_0.LD_en = 1;
	hps_tim_0.INT_en = 1;
	hps_tim_0.enable = 1;
	HPS_TIM_config_ASM(&hps_tim_0);

	//AUDIO TIMER
	HPS_TIM_config_t hps_tim_1;
	hps_tim_1.tim = TIM1;
	hps_tim_1.timeout = 20.83; //Audio speed 48000 samples / sec
	hps_tim_1.LD_en = 1;
	hps_tim_1.INT_en = 1;
	hps_tim_1.enable = 1;
	HPS_TIM_config_ASM(&hps_tim_1);

	//DISPLAY TIMER		
	HPS_TIM_config_t hps_tim_2;
	hps_tim_2.tim = TIM2;
	hps_tim_2.timeout = 7000;  //60 frames per seconds, however keeping it as that was much too slow, so we experimentally chose 7000
	hps_tim_2.LD_en = 1;
	hps_tim_2.INT_en = 1;
	hps_tim_2.enable = 1;
	HPS_TIM_config_ASM(&hps_tim_2);   
	}


/*CONTROL WAVES*/
int main() {
	VGA_clear_pixelbuff_ASM(); //Make sure to clear all buffers first
	VGA_clear_charbuff_ASM();
	setup_timers();
	while(1){
		handle_keyboard(); //Do the keyboard
		display_waves(); //Do the VGA displaying
	}
	return 0;
}

