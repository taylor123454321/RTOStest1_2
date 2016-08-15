/*
 * speed.h
 *
 *  Created on: 28/08/2015
 *      Author: Ryan Taylor
 */

#ifndef SPEED_H_
#define SPEED_H_

#include "init.h"

/* DEFINES */
#define BUF_SIZE 8
#define MAX_SET_SPEED 130
#define MIN_SET_SPEED 50


/* FUNCTIONS */

/* This function sets the speed for the car to stay at and make sure its with in the min and max bounds */
set_speed_s set_speed_func(set_speed_s set_speed, button_data_s button_data, float speed);

/* This function stores the Speed in a buffer for analysis later */
circBuf_t store_speed(float single_speed, circBuf_t speed_buffer);

/* Get speed from buffer */
float analysis_speed(circBuf_t speed_buffer);

/* Decode encoder interupt pin state change */
encoder_s encoder_quad(encoder_s encoder, encoder_raw_DATA_s encoder_raw_DATA);

#endif
