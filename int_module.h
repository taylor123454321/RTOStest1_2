/*
 * int_module.h
 *
 *  Created on: Aug 13, 2016
 *      Author: Ryan Taylor
 */

#ifndef INT_MODULE_H_
#define INT_MODULE_H_

/* Operated when char arrives on UART port*/
void UARTIntHandler(void);

/* Operates when a pin changes on PF5 or PF7 */
void EncoderINT (void);

#endif /* INT_MODULE_H_ */
