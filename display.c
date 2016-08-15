/*
 * display.c
 *
 *  Created on: 28/08/2015
 *      Author: Ryan Taylor
 */

#include <stdio.h>
#include <stdint.h>
#include "display.h"
#include "rit128x96x4.h"
#include "init.h"

/* This function clears the display */
void clearDisplay(void){
	RIT128x96x4Clear();
}

/* This function takes in button inputs and selects what screen to display */
set_speed_s read_button_screen(button_data_s button_data, set_speed_s set_speed, bool fix){
	int screen = set_speed.screen;
	//button_data_s button_data = return_button();

	if (fix == 0){
		screen = 4;//no fix screen
	}
	else {
		if (button_data.select == 1 || (fix == 1 && screen == 4)){// back function
			screen = 0;
		}
		if (button_data.left == 1 && screen == 0){ //acceleration
			screen = 2;
		}
		if ((button_data.up || button_data.down) == 1 && screen == 0){ // set speed
			screen = 1;
			set_speed.is_speed_set = 1;
		}
	}
	if (set_speed.screen != screen){
		clearDisplay();
	}
	set_speed.screen = screen;
	return set_speed;
}

/* Set speed screen */
void set_speed_display(int speed_set){
	char stringA[30];char stringB[30];char stringC[30];char stringD[30];char stringE[30];char stringF[30];char stringG[30];
	sprintf(stringA, "Set Speed Now!");
	sprintf(stringB, "%d  ",speed_set);
	sprintf(stringC, "  ");
	sprintf(stringD, "  ");
	sprintf(stringE, "  ");
	sprintf(stringF, "  ");
	sprintf(stringG, "  ");
	RIT128x96x4StringDraw (stringA, 6, 12, 15);
	RIT128x96x4StringDraw (stringB, 6, 24, 15);
	RIT128x96x4StringDraw (stringC, 6, 34, 15);
	RIT128x96x4StringDraw (stringD, 6, 46, 15);
	RIT128x96x4StringDraw (stringE, 6, 58, 15);
	RIT128x96x4StringDraw (stringF, 6, 70, 15);
	RIT128x96x4StringDraw (stringG, 6, 82, 15);
}

/* When no GPS fix is pressent */
void no_fix_screen(clock time, uint8_t satellite, int encoder, int aim_pos, PWM_DATA_s PWM_DATA){
	char stringA[30];char stringB[30];char stringC[30];char stringD[30];char stringE[30];char stringF[30];char stringG[30];
	sprintf(stringA, "Time %d.%d.%d.%d      ", time.hour, time.minute, time.seconds, time.milliseconds);
	sprintf(stringB, "There is no fix :(  ");
	sprintf(stringC, "Satellites  %d   ", satellite);
	sprintf(stringD, "PWM = %d, dir = %d ", PWM_DATA.duty, PWM_DATA.direction);
	sprintf(stringE, "Encoder %d    ", encoder);
	sprintf(stringF, "");//Aim pos  %d     ", aim_pos);
	sprintf(stringG, "");
	RIT128x96x4StringDraw (stringA, 6, 12, 15);
	RIT128x96x4StringDraw (stringB, 6, 24, 15);
	RIT128x96x4StringDraw (stringC, 6, 34, 15);
	RIT128x96x4StringDraw (stringD, 6, 46, 15);
	RIT128x96x4StringDraw (stringE, 6, 58, 15);
	RIT128x96x4StringDraw (stringF, 6, 70, 15);
	RIT128x96x4StringDraw (stringG, 6, 82, 15);
}

/* Main display function */
void display(int screen, float acc, float max_acc, int speed_set, GPS_DATA_DECODED_s DATA, float buffed_speed_,
		unsigned long encoder, float distance, char * stuff, int aim_pos, unsigned long adc, PWM_DATA_s PWM_DATA){
	if (screen == 1){
		set_speed_display(speed_set);
	}
	else if (screen == 4){
		no_fix_screen(DATA.real_time_s, DATA.satellites_s, encoder, aim_pos, PWM_DATA);
	}
	else {
		char stringA[30];char stringB[30];char stringC[30];char stringD[30];char stringE[30];char stringG[30];
		sprintf(stringA, "Speed %.1f  %d.%d.%d  ", buffed_speed_, DATA.real_time_s.hour, DATA.real_time_s.minute, DATA.real_time_s.seconds);
		sprintf(stringB, "Acc %.1f Max %.1f   ", acc, max_acc);
		sprintf(stringC, "Set speed %d    ", speed_set);
		sprintf(stringD, "Satellites %d   ", DATA.satellites_s);
		sprintf(stringE, "PWM = %d, dir = %d ", PWM_DATA.duty, PWM_DATA.direction);
		sprintf(stringG, "Int %d Enc %d   ", adc, encoder);
		//sprintf(stringF, "  ");
		//sprintf(stringG, "Distance  %.2f   ", distance);
		RIT128x96x4StringDraw (stringA, 6, 12, 15);
		RIT128x96x4StringDraw (stringB, 6, 24, 15);
		RIT128x96x4StringDraw (stringC, 6, 34, 15);
		RIT128x96x4StringDraw (stringD, 6, 46, 7);
		RIT128x96x4StringDraw (stringE, 6, 58, 7);
		RIT128x96x4StringDraw (stuff, 6, 70, 7);
		RIT128x96x4StringDraw (stringG, 6, 82, 15);
	}
}
