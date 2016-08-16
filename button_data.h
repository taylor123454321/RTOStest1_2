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
#include "inc/hw_memmap.h"

/* Reads GPIO pins connected to buttons */
button_data_raw_s read_buttons(void);

/* Inverts the button data so that 1 is pressed and 0 is not presed */
bool invert_bool(bool button);

/* Inverts all the button data */
button_data_s invert_buttons(button_data_raw_s raw_button_data);


#endif /* BUTTON_DATA_H_ */
