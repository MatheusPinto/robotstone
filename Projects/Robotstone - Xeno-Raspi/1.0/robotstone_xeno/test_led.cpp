/*
 * test_led.cpp
 *
 *  Created on: 30/10/2018
 *      Author: matheus
 */

#include "test_led.hpp"
#include "wiringPi.h"
#include <stdio.h>

void led_Init(int pin)
{
	 // Export the pin to the GPIO directory
       // FILE *fp = fopen("/sys/class/gpio/export","w");
       // fprintf(fp,"%d",3);
       // fclose(fp);
	wiringPiSetup();
	  	pinMode(pin, OUTPUT);

}

void led_TurnOff(int pin)
{
	digitalWrite(pin, LOW);
}


void led_TurnOn(int pin)
{
	digitalWrite(pin, HIGH);
}
