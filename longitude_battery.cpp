/*
 * Longitude battery level functions
 *
 * Moises Beato
 * February 2017
 */
#include "longitude.h"
#include "Arduino.h"

#define bat_pin A0                                               //analog pin 0

uint8_t voltage_percentage;
  
// battery icon chech variables
unsigned long previousMillis = 0;
unsigned int interval        = 5000;                             //interval to check battery status

void get_bat_level(void)
{
  int bat_read              = analogRead(bat_pin);               // analog pins are input by default, so no need to use pinMode
  float voltage_in_max      = 6.0;                               
	float voltage_actual      = bat_read *(voltage_in_max/930.0);  //scale up to 6v. Vmax at bat_pin = 3.3v, therefore (3.3/1023 = 3/930)
	float voltage_cutoff      = 5.25;                              // 5v LDO turns off 
	float voltage_operational = voltage_actual - voltage_cutoff;   //at Vmax is 0.75  
	voltage_percentage        = map(voltage_operational, 0, 0.75, 0, 100); //scale 0 to 100%
 	return;
}

void update_bat_level(void)
{
	//dispays battery icon an level on the upper right corner of the screen
	if (voltage_percentage > 75)
		show_bat_level_100();
   
	else if (voltage_percentage < 75 && voltage_percentage > 50)
		show_bat_level_75();
    
	else if (voltage_percentage < 50 && voltage_percentage > 25)
		show_bat_level_50();
    
	else if (voltage_percentage < 25 && voltage_percentage > 15)
		show_bat_level_25();
    
	else
		show_bat_level_15();
}

