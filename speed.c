/*
 * speed.c
 *
 *  Created on: 28/08/2015
 *      Author: Ryan Taylor
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include "speed.h"
#include "display.h"
#include "driverlib/uart.h"
#include "inc/hw_uart.h"
#include "inc/hw_memmap.h"
#include "data_process.h"
#include "button_data.h"
#include "init.h"

/* This function sets the speed for the car to stay at and make sure its with in the min and max bounds */
set_speed_s set_speed_func(set_speed_s set_speed, button_data_s button_data, float speed){
	if (set_speed.is_speed_set == 1){
		set_speed.set_speed_value = speed;
		set_speed.is_speed_set = 0;
	}
	else {
		if (button_data.up == 1){
			if (set_speed.set_speed_value >= MAX_SET_SPEED){      // Greater than 130 is capped
				set_speed.set_speed_value = MAX_SET_SPEED;
			}
			else if (set_speed.set_speed_value <= MIN_SET_SPEED){ // Less than 50 is capped
				set_speed.set_speed_value = MIN_SET_SPEED;
			}
			else{
				set_speed.set_speed_value ++;
			}
		}
		else if (button_data.down == 1){
			if (set_speed.set_speed_value <= MIN_SET_SPEED){      // Less than 50 is capped
				set_speed.set_speed_value = MIN_SET_SPEED;
			}
			else if (set_speed.set_speed_value >= MAX_SET_SPEED){ // Greater than 130 is capped
				set_speed.set_speed_value = MAX_SET_SPEED;
			}
			else {
				set_speed.set_speed_value --;
			}
		}
	}
	return set_speed;
}



/* This function stores the Speed in a buffer for analysis later */
circBuf_t store_speed(float single_speed, circBuf_t speed_buffer){
	if (single_speed > 150 ){
		single_speed = 150;
	}
	speed_buffer.data[speed_buffer.windex] = single_speed;
	speed_buffer.windex ++;
	if (speed_buffer.windex >= speed_buffer.size){
		speed_buffer.windex = 0;
	}
	return speed_buffer;
}

/* Get speed from buffer */
float analysis_speed(circBuf_t speed_buffer){
	int i = 0;
	float speed_sum = 0;
	for (i = 0; i < BUF_SIZE; i++)
	speed_sum += speed_buffer.data[i];
	return (speed_sum/BUF_SIZE);
}

/* Decode encoder interupt pin state change */
encoder_s encoder_quad(encoder_s encoder, encoder_raw_DATA_s encoder_raw_DATA){
	int current_state = 0;
	if (!encoder_raw_DATA.ul_A_Val){       //Check what state the pins at and assign that state to "current state"
		if(!encoder_raw_DATA.ul_B_Val){
			current_state = 1;
		}
		else{
			current_state = 2;
		}
	}
	else{
		if(encoder_raw_DATA.ul_B_Val){
			current_state = 3;
		}
		else{
			current_state = 4;
		}
	}
	// Check if the previous state is different from the current state.
	// Determine what direction the encoder is spinning
	if (current_state != encoder.prev_state){
		if (abs(encoder.prev_state - current_state) == 1){
			if(current_state > encoder.prev_state){
				encoder.angle --;
			}
			else{
				encoder.angle ++;
			}
		}
		else{
			if(current_state < encoder.prev_state){
				encoder.angle --;
			}
			else{
				encoder.angle ++;
			}
		}
	}
	encoder.prev_state = current_state; // Assign current state for next time the interrupt runs
	return encoder;
}

