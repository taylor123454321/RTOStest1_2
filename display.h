/*
 * display.h
 *
 *  Created on: 28/08/2015
 *      Author: Ryan Taylor
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_


#include <stdint.h>
#include "init.h"

/* This function clears the display */
void clearDisplay(void);

/* This function takes in button inputs and selects what screen to display */
set_speed_s read_button_screen(button_data_s button_data, set_speed_s set_speed, bool fix);

/* Set speed screen */
void set_speed_display(int speed_set);

/* When no GPS fix is pressent */
void no_fix_screen(clock time, uint8_t satellite, int encoder, int aim_pos, PWM_DATA_s PWM_DATA);

/* Main display function */
void display(int screen, float acc, float max_acc, int speed_set, GPS_DATA_DECODED_s DATA, float buffed_speed,
		unsigned long encoder, float distance, char * stuff, int aim_pos, unsigned long adc, PWM_DATA_s PWM_DATA);

#endif /* DISPLAY_H_ */
