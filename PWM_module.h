/*
 * PWM_module.h
 *
 *  Created on: 13/07/2016
 *      Author: Ryan Taylor
 */

#ifndef PWM_MODULE_H_
#define PWM_MODULE_H_

#include <stdint.h>
#include "init.h"

/* Based on PWM duty this function picks the direction of the motor */
int find_dir(int aim_pos);

/* This function connects speed to carb/rpm for the two control loops of the program*/
PWM_DATA_s speed_feedback(PWM_speed_DATA_s PWM_speed_DATA, encoder_s encoder_1, PWM_DATA_s PWM_DATA);

/* Set duty cycle */
void PWM_duty(PWM_DATA_s PWM_DATA, unsigned long period);

/* Sets the pin high or low for direction of motor */
void PWM_direction(PWM_DATA_s PWM_DATA);


#endif /* PWM_MODULE_H_ */
