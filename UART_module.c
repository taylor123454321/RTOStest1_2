/*
 * UART_module.c
 *
 *  Created on: 30/08/2015
 *      Author: Ryan Taylor
 */

#include "UART_module.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_uart.h"
#include "driverlib/uart.h"


/* UART Send data, used in conjuction with send_data() */
void UARTSend(const unsigned char *pucBuffer, unsigned long ulCount, int type) {
    //
    // Loop while there are more characters to send.
    //
	long base = 0;
	if(type == 0){
		base = UART0_BASE;
	}
	else if(type == 1){
		base = UART1_BASE;
	}
    while(ulCount--) {
        //
        // Write the next character to the UART.
        //
    	UARTCharPut(base, *pucBuffer++);
    }
}

/* Send instruction to the GPS to set up how to frequency and serial data outputs */
void send_data(void){
	int i = 0;
	while(i <= 100000){i++;}
	i = 0;
	UARTSend((unsigned char *)PMTK_SET_NMEA_UPDATE_5HZ, 18, 0); // Set frequency

	while(i <= 100000){i++;}
	i = 0;
	UARTSend((unsigned char *)PMTK_SET_NMEA_OUTPUT_RMCGGA, 53,0); // Set receaving for only RMC and GGA

	while(i <= 1000000){i++;}
	UARTSend((unsigned char *)PGCMD_NOANTENNA, 16,0); // Set no antenna
}

