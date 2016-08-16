/*
 * UART_module.h
 *
 *  Created on: 30/08/2015
 *      Author: Ryan Taylor
 */

#ifndef UART_MODULE_H_
#define UART_MODULE_H_

#include <stdint.h>


/* DEFINES */
#define false 0
#define true 1

/* NEMA sentances for GPS data */
#define PMTK_SET_NMEA_UPDATE_5HZ  "$PMTK220,200*2C\r\n"         // Frequency 5Hz
#define PMTK_SET_NMEA_UPDATE_10HZ "$PMTK220,100*2F\r\n"         // Frequency 10Hz
#define PMTK_SET_NMEA_OUTPUT_RMCGGA "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n"// turn on GPRMC and GGA
// request for updates on antenna status
#define PGCMD_ANTENNA "$PGCMD,33,1*6C\r\n"
#define PGCMD_NOANTENNA "$PGCMD,33,0*6D\r\n"



/* FUNCTIONS */

/* UART Send data, used in conjuction with send_data() */
void UARTSend(const unsigned char *pucBuffer, unsigned long ulCount, int type);

/* Send instruction to the GPS to set up how to frequency and serial data outputs */
void send_data(void);

#endif /* DATA_PROCESS_H_ */
