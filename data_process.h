/*
 * data_process.h
 *
 *  Created on: 30/08/2015
 *      Author: Ryan Taylor
 */

#ifndef DATA_PROCESS_H_
#define DATA_PROCESS_H_

#include <stdint.h>
#include "init.h"

/* Put new char from GPS in string */
char* store_char(long UART_character, char * UART_char_data_old_2);

/* Decode fucntion for GGA NEMA sentence */
GPS_DATA_DECODED_s decode_GGA(char *p, GPS_DATA_DECODED_s DATA);

/* Decode fucntion for RMC NEMA sentence */
GPS_DATA_DECODED_s decode_RMC(char *p, GPS_DATA_DECODED_s DATA);

/* Split data up into GGA, GSA, RMC, VTG and only process GGA and RMC */
GPS_DATA_DECODED_s split_data(char *data_incoming, GPS_DATA_DECODED_s GPS_DATA_DECODED);

#endif /* DATA_PROCESS_H_ */
