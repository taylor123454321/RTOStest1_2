/*
 * data_process.h
 *
 *  Created on: 30/08/2015
 *      Author: Ryan Taylor
 */

#ifndef UART_MODULE_H_
#define UART_MODULE_H_

#include <stdint.h>

void UARTSend(const unsigned char *pucBuffer, unsigned long ulCount, int type);

void send_data(void);

#endif /* DATA_PROCESS_H_ */
