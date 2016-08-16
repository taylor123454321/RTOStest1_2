/*
 * button_data.h
 *
 *  Created on: 29/08/2015
 *      Author: Ryan Taylor
 */

#ifndef BUTTON_DATA_H_
#define BUTTON_DATA_H_

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "init.h"

/* Read GPIO pins for buttons */
button_data_raw_s read_buttons(void);

/* Invert bool for button value inverting */
bool invert_bool(bool button);

/* Invert all buttons using inver_bool() */
button_data_s invert_button(button_data_raw_s raw_button_data);

#endif /* BUTTON_DATA_H_ */
