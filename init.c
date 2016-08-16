/*
 * init.c
 *
 *  Created on: May 13, 2015
 *      Author: Ryan Taylor
 */

#include <stdbool.h>
#include "stdlib.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "inc/hw_uart.h"
#include "inc/hw_memmap.h"
#include "driverlib/adc.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "rit128x96x4.h"
#include "FreeRTOSConfig.h"
#include "UART_module.h"
#include "init.h"

// Defines for the GPS module

// Global constants
unsigned long period;	// Period of PWM output.

//*****************************************************************************
// Initialization functions
//*****************************************************************************

/* Clears all the peripheral used */
void reset_peripheral(void){
	SysCtlPeripheralReset (SYSCTL_PERIPH_PWM);
	SysCtlPeripheralReset(SYSCTL_PERIPH_GPIOG);
	SysCtlPeripheralReset(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralReset(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralReset(SYSCTL_PERIPH_ADC0);
	SysCtlPeripheralReset(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralReset(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralReset(SYSCTL_PERIPH_TIMER0);
	SysCtlPeripheralReset(SYSCTL_PERIPH_TIMER1);
}

/* Sets up the proccesor speed */
void initClock (void) {
	/* Set the clocking to run from the PLL at 50 MHz.  Assumes 8MHz XTAL,
	whereas some older eval boards used 6MHz. */
  	SysCtlClockSet( SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ );

}

/* Enable GPIO for buttons and motor drive pins */
void initGPIO(void){
    SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOG);
    GPIOPadConfigSet (GPIO_PORTG_BASE, GPIO_PIN_7 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3 | GPIO_PIN_1 );
}

/* Init for encoder pin interupts, PF5 and PF7 */
void initPin (void) {
    // Register the handler for Port F into the vector table
    //GPIOPortIntRegister (GPIO_PORTF_BASE, PinChangeIntHandler);
    //
    // Enable and configure the port and pin used:  input on PF5: Pin 27
    SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOF);
    GPIOPadConfigSet (GPIO_PORTF_BASE, GPIO_PIN_5, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPU);
    //
    // Set up the pin change interrupt (both edges)
    GPIOIntTypeSet (GPIO_PORTF_BASE, GPIO_PIN_5, GPIO_BOTH_EDGES);
    //
    // Enable the pin change interrupt
    GPIOPinIntEnable (GPIO_PORTF_BASE, GPIO_PIN_5);


    GPIOPadConfigSet (GPIO_PORTF_BASE, GPIO_PIN_7, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPU);
    //
    // Set up the pin change interrupt (both edges)
    GPIOIntTypeSet (GPIO_PORTF_BASE, GPIO_PIN_7, GPIO_BOTH_EDGES);

    IntPrioritySet( INT_GPIOF, configKERNEL_INTERRUPT_PRIORITY);

    //
    // Enable the pin change interrupt
    GPIOPinIntEnable (GPIO_PORTF_BASE, GPIO_PIN_7);

    IntEnable (INT_GPIOF);	// Note: INT_GPIOF defined in inc/hw_ints.h
}


/* Init the OLED display */
void initDisplay (void) {
	RIT128x96x4Init(1000000);
}

/* Initlise the PWM for pin PWM4. This sets up the period and frequecy also */
void initPWMchan(void) {
	period = SysCtlClockGet () / PWM_DIVIDER / PWM4_RATE_HZ;

    SysCtlPeripheralEnable (SYSCTL_PERIPH_PWM);


    SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOF);
    GPIOPinTypePWM (GPIO_PORTF_BASE, GPIO_PIN_2);

    SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOD);
    GPIOPinTypePWM (GPIO_PORTD_BASE, GPIO_PIN_1);

    //
    // Compute the PWM period based on the system clock.
    //
    SysCtlPWMClockSet (PWM_DIV_CODE);
    PWMGenConfigure (PWM_BASE, PWM_GEN_2, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet (PWM_BASE, PWM_GEN_2, period);
    PWMPulseWidthSet (PWM_BASE, PWM_OUT_4, period * PWM4_DEF_DUTY / 100);

    PWMGenConfigure (PWM_BASE, PWM_GEN_0, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet (PWM_BASE, PWM_GEN_0, period);
    PWMPulseWidthSet (PWM_BASE, PWM_OUT_1, period * PWM4_DEF_DUTY / 100);
    //
    // Enable the PWM generator.
    //
    PWMGenEnable (PWM_BASE, PWM_GEN_2);
    PWMGenEnable (PWM_BASE, PWM_GEN_0);
    //
    // Enable the PWM output signal.
    //
    PWMOutputState (PWM_BASE, PWM_OUT_4_BIT, true);
    PWMOutputState (PWM_BASE, PWM_OUT_1_BIT, true);
}



//***********************************************************
// Initialize UART0 - 8 bits, 1 stop bit, no parity
//***********************************************************
void initConsole (void) {
	//
	// Enable GPIO port A which is used for UART0 pins.
	//
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);// normal
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);// extra
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    //
    // Enable processor interrupts.
    //

	//
	// Select the alternate (UART) function for these pins.
	//
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);//normal
	GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_2 | GPIO_PIN_3);//extra
	//
	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(),
			BAUD_RATE_GPS, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
			UART_CONFIG_PAR_NONE));//normal

	UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(),
				BAUD_RATE_GPS, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
				UART_CONFIG_PAR_NONE));//extra for fake gps

    IntPrioritySet( INT_UART0, configKERNEL_INTERRUPT_PRIORITY);
    //
    // Enable the UART interrupt.
    //
    IntEnable(INT_UART0);
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    
	//
	UARTFIFOEnable(UART1_BASE);
	UARTEnable(UART1_BASE);
}

/* Runs main init for program */
void main_init(void){
    IntMasterDisable(); // Disable interupts while init is happing
    reset_peripheral(); // Reset all peripheral
    initClock();        // Enable clock
    initPin();          // Enable pin change interupt for encoder
    initGPIO();         // Enable button and motor direction GPIO
    initConsole();      // Enable UART0 for GPS data
    initPWMchan();      // Enable PWM for motor control
    initDisplay();      // Enable the OLED display
    send_data();send_data();send_data(); // Send data to GPS three time to make sure it gets it
}

// *******************************************************
// initCircBuf: Initialise the circBuf instance. Reset both indices to
// the start of the buffer.  Dynamically allocate and clear the the
// memory and return a pointer for the data.  Return NULL if
// allocation fails.
// *******************************************************
float * initCircBuf (circBuf_t *buffer, unsigned int size) {
	buffer->windex = 0;
	buffer->rindex = 0;
	buffer->size = size;
	buffer->data =
        (float *) calloc (size, sizeof(float));
     // Note use of calloc() to clear contents.
	return buffer->data;
}

/* Sets up struct with zeros */
void init_set_speed_data(cruise_data *data){
	data->speed = 0;
	data->enable = 0;
	data->old = 0;
}


