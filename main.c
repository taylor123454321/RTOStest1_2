/*
 *  	Cruise control program
 *  	Created on: May 13, 2015
 *  	Last changed on: Aug 15, 2016
 *      Author: Ryan Taylor
*/
/* C includes */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

/* FreeRTOS includes. */
#include "include/FreeRTOS.h"
#include "include/task.h"
#include "include/semphr.h"

/* Stellaris library includes. */
#include "inc\hw_types.h"
#include "inc\hw_memmap.h"
#include "inc/hw_uart.h"
#include "driverlib/sysctl.h"
#include "rit128x96x4.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"

/* User made library includes */
#include "init.h"
#include "data_process.h"
#include "display.h"
#include "speed.h"
#include "demo_code\basic_io.h"
#include "button_data.h"
#include "PWM_module.h"
#include "int_module.h"
#include "UART_module.h"

#define BUF_SIZE 8

/* The task function. */
void vReadGPS( void *pvParameters );
void vDisplayTask( void *pvParameters );
void vReadButtons( void *pvParameters );
void vFilterSpeed( void *pvParameters );
void vEncoder( void *pvParameters );
void vPWM( void *pvParameters );
void vReadGPSchar( void *pvParameters );

//Global variables
char UART_char_data_old[120];

// RTOS semaphore handles
xSemaphoreHandle  xBinarySemaphoreGPS;
xSemaphoreHandle  xBinarySemaphoreGPSchar;
xSemaphoreHandle  xBinarySemaphoreFilter;
xSemaphoreHandle  xBinarySemaphoreEncoder_1;

/* RTOS Queuehandles */
xQueueHandle xQueueGPSDATA;
xQueueHandle xQueueButtons;
xQueueHandle xQueueSpeed;
xQueueHandle xQueueBuffedSpeed;
xQueueHandle xEncoder_1;
xQueueHandle xPWM_DATA;
xQueueHandle xPWM_speed_DATA;
xQueueHandle xEncoder_raw_DATA;
xQueueHandle xUART_GPS_DATA;

/*-----------------------------------------------------------*/
int main( void ) {
    main_init();
	
    /* Creates Semaphores */
	vSemaphoreCreateBinary(xBinarySemaphoreGPS);
	vSemaphoreCreateBinary(xBinarySemaphoreGPSchar);
	vSemaphoreCreateBinary(xBinarySemaphoreFilter);
	vSemaphoreCreateBinary(xBinarySemaphoreEncoder_1);

    /* Creates Queues */
	xQueueGPSDATA     = xQueueCreate( 1, sizeof(GPS_DATA_DECODED_s));
	xQueueButtons     = xQueueCreate( 1, sizeof(button_data_s));
	xQueueSpeed       = xQueueCreate( 1, sizeof(float));
	xQueueBuffedSpeed = xQueueCreate( 1, sizeof(float));
	xEncoder_1        = xQueueCreate( 1, sizeof(encoder_s));
	xPWM_DATA         = xQueueCreate( 1, sizeof(PWM_DATA_s));
	xPWM_speed_DATA   = xQueueCreate( 1, sizeof(PWM_speed_DATA_s));
	xEncoder_raw_DATA = xQueueCreate( 1, sizeof(PWM_DATA_s));
	xUART_GPS_DATA    = xQueueCreate( 1, sizeof(UART_GPS_DATA_s));

	/* Create tasks. */
	xTaskCreate( vReadGPS,     "GPS Read Task",     240, NULL, 4, NULL );
    xTaskCreate( vReadGPSchar, "ReadGPSchar Task",  250, NULL, 4, NULL);
	xTaskCreate( vPWM,         "PWM Task",          100, NULL, 3, NULL );
    xTaskCreate( vEncoder,     "Encoder Task",      100, NULL, 3, NULL );
    xTaskCreate( vFilterSpeed, "Filter Speed Task", 100, NULL, 2, NULL );
    xTaskCreate( vReadButtons, "Read Buttons Task", 150, NULL, 2, NULL );
    xTaskCreate( vDisplayTask, "Display Task",      600, NULL, 1, NULL );


	IntMasterEnable();

	/* Start the scheduler so our tasks start executing. */
	vTaskStartScheduler();	
	while(1);
}

/*Tasks for Program */
/*-----------------------------------------------------------*/
/* Run to decode GPS NEMA sentence */
void vReadGPSchar(void *pvParameters){
	xSemaphoreTake(xBinarySemaphoreGPSchar, 0);

    while(1) { // Loop while there are characters in the receive FIFO.
		while(UARTCharsAvail(UART0_BASE)) {
			long UART_char = UARTCharGetNonBlocking(UART0_BASE);
			strcpy(UART_char_data_old, store_char(UART_char, UART_char_data_old));

		}
		xSemaphoreTake(xBinarySemaphoreGPSchar, portMAX_DELAY);                  // Take original semaphore
	}
}

/* Run to decode GPS NEMA sentence */
void vReadGPS(void *pvParameters){
	xSemaphoreTake(xBinarySemaphoreGPS, 0);
	GPS_DATA_DECODED_s GPS_DATA_DECODED;                                     // All data from GPS
	//UART_GPS_DATA_s UART_DATA;

	while(1) {
		//xQueueReceive(xUART_GPS_DATA, &UART_DATA, 0);
		GPS_DATA_DECODED = split_data(UART_char_data_old, GPS_DATA_DECODED); // Decode NEMA sentence
		xQueueSendToBack(xQueueGPSDATA, &GPS_DATA_DECODED, 0);               // Store data in Queue

		xSemaphoreGive(xBinarySemaphoreFilter);                              // Tell filter task to run
		xQueueSendToBack(xQueueSpeed, &GPS_DATA_DECODED.speed_s, 0);         // Send speed to filter task
		xSemaphoreTake(xBinarySemaphoreGPS, portMAX_DELAY);                  // Take original semaphore
	}
}

/* Task to read buttons and store data */
void vReadButtons(void *pvParameters){
	button_data_s button_data;
	button_data_raw_s raw_button_data;                    // All button data
    
	while(1){
		raw_button_data = read_buttons();                 // Read buttons
		button_data = invert_buttons(raw_button_data);    // Invert button data
		xQueueSendToBack(xQueueButtons, &button_data, 0); // Store button data

		vTaskDelay(14 / portTICK_RATE_MS);                // Set Read button task to run at 75Hz
	}
}

/* Task to run after pin change interupt to decode data for encoder */
void vEncoder(void *pvParameters){
    xSemaphoreTake(xBinarySemaphoreEncoder_1, 0);
	encoder_raw_DATA_s encoder_raw_DATA;
	encoder_s encoder_1;
	encoder_1.angle = 0;

	while(1){
		xQueueReceive(xEncoder_raw_DATA, &encoder_raw_DATA, 0);   // Get pin change data
		encoder_1 = encoder_quad(encoder_1, encoder_raw_DATA);    // Decode pin change and update encoder position

		xQueueSendToBack(xEncoder_1, &encoder_1, 0);              // Store button data
		xSemaphoreTake(xBinarySemaphoreEncoder_1, portMAX_DELAY); // Take orginial semaphore
	}
}

/* Task to update PWM and calculate control of motor positon */
void vPWM(void *pvParameters){
	unsigned long period = SysCtlClockGet () / PWM_DIVIDER / PWM4_RATE_HZ;	// Period of PWM output.
	encoder_s encoder_1;             // Encoder position
	PWM_DATA_s PWM_DATA;             // PWM duty cycle and direction
	PWM_speed_DATA_s PWM_speed_DATA; // Speed and set speed data used for controlling speed of car

	PWM_DATA.duty = 0;
	PWM_DATA.direction = 0;

	while(1){
		xQueueReceive(xEncoder_1, &encoder_1, 0);
		xQueueReceive(xPWM_speed_DATA, &PWM_speed_DATA, 0);				// Get data from queues

		PWM_DATA = speed_feedback(PWM_speed_DATA, encoder_1, PWM_DATA); // Function to run two closed loop control circuits
		PWM_direction(PWM_DATA);                                        // Update PWM perhferial
		PWM_duty(PWM_DATA, period);
		xQueueSendToBack(xPWM_DATA, &PWM_DATA, 0);                      // Store data in queue for display
		vTaskDelay(33 / portTICK_RATE_MS);                              // Set PWM task to run at 30Hz
	}
}

/* Task to filter speed in circular buffer */
void vFilterSpeed( void *pvParameters ){
	circBuf_t speed_buffer;
	initCircBuf (&speed_buffer, BUF_SIZE);                        // Init buffer
	xSemaphoreTake(xBinarySemaphoreFilter, 0);                    // Take inital semaphore
	float speed_to_store = 0;
	float buffed_speed = 0;

	while(1){
		xQueueReceive(xQueueSpeed, &speed_to_store, 0);           // Receive speed from queue
		speed_buffer = store_speed(speed_to_store, speed_buffer); // Put new value in buffer
		buffed_speed = analysis_speed(speed_buffer);              // Average buffer
		xQueueSendToBack(xQueueBuffedSpeed, &buffed_speed, 0);    // Store value in Queue
		xSemaphoreTake(xBinarySemaphoreFilter, portMAX_DELAY);
	}
}

/* Task to run display */
void vDisplayTask( void *pvParameters ){
	GPS_DATA_DECODED_s GPS_DATA_DECODED; // Decoded GPS data
	button_data_s button_data;           // Button data
	float buffed_speed = 0;              // Filtered speed
	set_speed_s set_speed;               // Set speed struct
	encoder_s encoder_1;
	PWM_DATA_s PWM_DATA;
    PWM_speed_DATA_s PWM_speed_DATA;     // Speed and set speed data used for controlling speed of car
	set_speed.set_speed_value = 80;      // Init set speed struct
	set_speed.is_speed_set = 0;
	set_speed.screen = 0;

	while(1) {
		xQueueReceive(xQueueGPSDATA, &GPS_DATA_DECODED, 0); // Recive data from various queues
		xQueueReceive(xQueueButtons, &button_data, 0);
		xQueueReceive(xQueueBuffedSpeed, &buffed_speed, 0);
		xQueuePeek(xEncoder_1, &encoder_1, 0);
		xQueueReceive(xPWM_DATA, &PWM_DATA, 0);

		set_speed = read_button_screen(button_data,set_speed, 1);         // Read buttons for selecting screen/state of program
		set_speed = set_speed_func(set_speed, button_data, buffed_speed); // Seting the speed you want to cruise at
		PWM_speed_DATA.set_speed = set_speed.set_speed_value;             // Transfering data for PWM control
		PWM_speed_DATA.speed = GPS_DATA_DECODED.speed_s;

		xQueueSendToBack(xPWM_speed_DATA, &PWM_speed_DATA, 0);            // Sending that data in queue

		display(set_speed.screen, 0, 0, set_speed.set_speed_value, GPS_DATA_DECODED, buffed_speed, encoder_1.angle, 0, UART_char_data_old, 0, 0, PWM_DATA); // Main display function

		vTaskDelay(66 / portTICK_RATE_MS);                                // Set display task to run at 15Hz
	}
}

/*-----------------------------------------------------------*/
void vApplicationMallocFailedHook( void )
{
	/* This function will only be called if an API call to create a task, queue
	or semaphore fails because there is too little heap RAM remaining. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed char *pcTaskName )
{
	/* This function will only be called if a task overflows its stack.  Note
	that stack overflow checking does slow down the context switch
	implementation. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* This example does not use the idle hook to perform any processing. */
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
	/* This example does not use the tick hook to perform any processing. */
}


