/*
 * data_process.c
 *
 *  Created on: 30/08/2015
 *      Author: Ryan Taylor
 */
//INCLUDEDS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "init.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "data_process.h"
#include "include/FreeRTOS.h"
#include "include/semphr.h"

#define false 0
#define true 1
#define MAX_24BIT_VAL 0X0FFFFFF

/* freeRTOS queues and semaphore */
xQueueHandle xUART_GPS_DATA;
xSemaphoreHandle  xBinarySemaphoreGPS;

/* Put new char from GPS in string */
char* store_char(long UART_character, char * UART_char_data_old_2){
	UART_GPS_DATA_s UART_DATA;

	xQueueReceive(xUART_GPS_DATA, &UART_DATA, 0);

	if (UART_character == '$'){
		UART_char_data_old_2[0] = '\0';
		strcpy(UART_char_data_old_2, UART_DATA.UART_char_data);
		UART_DATA.index = 0;
		UART_DATA.UART_char_data[UART_DATA.index] = UART_character;
		UART_DATA.index ++;

		xQueueSendToBack(xUART_GPS_DATA, &UART_DATA, 0);
		xSemaphoreGive(xBinarySemaphoreGPS);
	}
	else{
		UART_DATA.UART_char_data[UART_DATA.index] = UART_character;
		UART_DATA.index ++;

		xQueueSendToBack(xUART_GPS_DATA, &UART_DATA, 0);
	}
	return UART_char_data_old_2;
}

/* Decode fucntion for GGA NEMA sentence */
GPS_DATA_DECODED_s decode_GGA(char *p, GPS_DATA_DECODED_s DATA){
	int32_t degree;
	long minutes;
	char degreebuff[10];
	clock real_time;
	long_lat location;
	float latitude = 0;
	float longitude = 0;
	uint32_t latitudeDegrees = 0;
	uint32_t longitudeDegrees = 0;
	char lat, lon;

	// GET TIME
	p = strchr(p, ',')+1;
	float timef = atof(p);
	uint32_t time = timef;
	real_time.hour = time / 10000;
	real_time.minute = (time % 10000) / 100;
	real_time.seconds = (time % 100);
	real_time.milliseconds = fmod(timef, 1.0) * 1000;
	DATA.real_time_s = real_time;

	// GET LATITUDE
	p = strchr(p, ',')+1;
	if (',' != *p) {
		strncpy(degreebuff, p, 2);
		p += 2;
		degreebuff[2] = '\0';
		degree = atol(degreebuff) * 10000000;
		strncpy(degreebuff, p, 2); // minutes
		p += 3; // skip decimal point
		strncpy(degreebuff + 2, p, 4);
		degreebuff[6] = '\0';
		minutes = 50 * atol(degreebuff) / 3;
		location.latitude_s = degree + minutes;
		latitude = degree / 100000 + minutes * 0.000006F;
		int convert = latitude/100;
		latitudeDegrees = (latitude - 100 * convert)/60.0 ;
		latitudeDegrees += convert;
	}

	p = strchr(p, ',')+1;
	if (',' != *p) {
		if (p[0] == 'S') latitudeDegrees *= -1.0;
	    if (p[0] == 'N') lat = 'N';
	    else if (p[0] == 'S') lat = 'S';
	    else if (p[0] == ',') lat = 0;
	    //else return false;
	}

	// GET LONGDITUDE
	p = strchr(p, ',')+1;
	if (',' != *p) {
		strncpy(degreebuff, p, 3);
	    p += 3;
	    degreebuff[3] = '\0';
	    degree = atol(degreebuff) * 10000000;
	    strncpy(degreebuff, p, 2); // minutes
	    p += 3; // skip decimal point
	    strncpy(degreebuff + 2, p, 4);
	    degreebuff[6] = '\0';
	    minutes = 50 * atol(degreebuff) / 3;
	    location.longitude_s = degree + minutes;
	    longitude = degree / 100000 + minutes * 0.000006F;
	    int convert_2 = longitude/100;
	    longitudeDegrees = (longitude - 100*convert_2)/60.0;
	    longitudeDegrees += convert_2;
	}
	DATA.location_s = location;

	p = strchr(p, ',')+1;
	if (',' != *p){
		if (p[0] == 'W') longitudeDegrees *= -1.0;
	    if (p[0] == 'W') lon = 'W';
	    else if (p[0] == 'E') lon = 'E';
	    else if (p[0] == ',') lon = 0;
	    //else return false;
	}
	p = strchr(p, ',')+1;
	if (',' != *p){
		DATA.fixquality_s = atoi(p);
	}

	p = strchr(p, ',')+1;
	if (',' != *p){
		DATA.satellites_s = atoi(p);
	}

	p = strchr(p, ',')+1;
	if (',' != *p){
	    DATA.HDOP_s = atof(p);
	}

	p = strchr(p, ',')+1;
	if (',' != *p){
	    DATA.altitude_s = atof(p);
	}

	p = strchr(p, ',')+1;
	p = strchr(p, ',')+1;
	if (',' != *p){
	    DATA.geoidheight_s = atof(p);
	}
	return DATA;
}

/* Decode fucntion for RMC NEMA sentence */
GPS_DATA_DECODED_s decode_RMC(char *p, GPS_DATA_DECODED_s DATA){
	int32_t degree;
	long minutes;
	char degreebuff[10];

	clock real_time;
	float longitude = 0;
	float latitude = 0;
	uint32_t latitudeDegrees = 0;
	uint32_t longitudeDegrees = 0;
	char lat, lon;
	int32_t longitude_fixed = 0;
	int32_t latitude_fixed = 0;
	bool fix = 0;

	// get time
	p = strchr(p, ',')+1;
	float timef = atof(p);
	uint32_t time = timef;
	real_time.hour = time / 10000;
	real_time.minute = (time % 10000) / 100;
	real_time.seconds = (time % 100);
	real_time.milliseconds = fmod(timef, 1.0) * 1000;

	p = strchr(p, ',')+1;
	// Serial.println(p);
	if (p[0] == 'A')
	    fix = true;
	else if (p[0] == 'V')
	    fix = false;
	//else
	//    return false;
	DATA.fix_s = fix;

	// parse out latitude
	p = strchr(p, ',')+1;
	if (',' != *p){
	    strncpy(degreebuff, p, 2);
	    p += 2;
	    degreebuff[2] = '\0';
	    long degree = atol(degreebuff) * 10000000;
	    strncpy(degreebuff, p, 2); // minutes
	    p += 3; // skip decimal point
	    strncpy(degreebuff + 2, p, 4);
	    degreebuff[6] = '\0';
	    long minutes = 50 * atol(degreebuff) / 3;
	    latitude_fixed = degree + minutes;
	    latitude = degree / 100000 + minutes * 0.000006F;
	    int convert = latitude/100;
	    latitudeDegrees = (latitude - 100*convert)/60.0;
	    latitudeDegrees += convert;
	}

	p = strchr(p, ',')+1;
	if (',' != *p){
	    if (p[0] == 'S') latitudeDegrees *= -1.0;
	    if (p[0] == 'N') lat = 'N';
	    else if (p[0] == 'S') lat = 'S';
	    else if (p[0] == ',') lat = 0;
	    //else return false;
	}

	// parse out longitude
	p = strchr(p, ',')+1;
	if (',' != *p){
	    strncpy(degreebuff, p, 3);
	    p += 3;
	    degreebuff[3] = '\0';
	    degree = atol(degreebuff) * 10000000;
	    strncpy(degreebuff, p, 2); // minutes
	    p += 3; // skip decimal point
	    strncpy(degreebuff + 2, p, 4);
	    degreebuff[6] = '\0';
	    minutes = 50 * atol(degreebuff) / 3;
	    longitude_fixed = degree + minutes;
	    longitude = degree / 100000 + minutes * 0.000006F;
	    int convert_2 = longitude/100;
	    longitudeDegrees = (longitude-100*convert_2)/60.0;
	    longitudeDegrees += convert_2;
	}

	p = strchr(p, ',')+1;
	if (',' != *p){
	    if (p[0] == 'W') longitudeDegrees *= -1.0;
	    if (p[0] == 'W') lon = 'W';
	    else if (p[0] == 'E') lon = 'E';
	    else if (p[0] == ',') lon = 0;
	    //else return false;
	}
	// speed
	p = strchr(p, ',')+1;
	if (',' != *p){
		DATA.speed_s = atof(p);
	}

	// angle
	p = strchr(p, ',')+1;
	if (',' != *p){
	    DATA.angle_s = atof(p);
	}

	p = strchr(p, ',')+1;
	if (',' != *p){
	    uint32_t fulldate = atof(p);
	    real_time.day = fulldate / 10000;
	    real_time.month = (fulldate % 10000) / 100;
	    real_time.year = (fulldate % 100);
	}
	return DATA;
}

/* Split data up into GGA, GSA, RMC, VTG and only process GGA and RMC */
GPS_DATA_DECODED_s split_data(char *data_incoming, GPS_DATA_DECODED_s GPS_DATA_DECODED){
	if (strstr(data_incoming, "GGA") != NULL){
		GPS_DATA_DECODED = decode_GGA(data_incoming, GPS_DATA_DECODED);
	}
	else if (strstr(data_incoming, "RMC") != NULL){
		GPS_DATA_DECODED = decode_RMC(data_incoming, GPS_DATA_DECODED);
	}
	else if (strstr(data_incoming, "GSA") != NULL){
	}
	else if (strstr(data_incoming, "VTG") != NULL){
	}

	return GPS_DATA_DECODED;
}

















