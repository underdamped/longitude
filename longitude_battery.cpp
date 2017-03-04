/*
 * Longitude battery level functions
 *
 * Moises Beato, Javier Lombillo
 * February 2017
 */
#include "longitude.h"
#include "Arduino.h"

#define bat_pin A0   // analog pin 0

// we measure battery level using the internal Teensy ADC at 10-bit resolution (vref = 3.3V).
// the nominal 6V battery voltage is dropped to 3V before the ADC pin, which corresponds to
// an ADC count of 931.  at maximum load, our drop-out voltage is 5.25V, which equals 2.625V
// after the voltage divider. this corresponds to 815 counts, our minimum battery value.
#define BATTERY_MAX    (931)
#define BATTERY_MIN    (815)
#define BATTERY_OFFSET (31) // -100 mV of offset error

uint8_t voltage_percentage;

void get_bat_level(void)
{
  uint16_t bat_read = analogRead(bat_pin); // analog pins are input by default, so no need to use pinMode
  bat_read += BATTERY_OFFSET;

  voltage_percentage = map(bat_read, BATTERY_MIN, BATTERY_MAX, 0, 100); // scale 0 to 100%
  
 	return;
}

void update_bat_level(void)
{
	get_bat_level();
  
	// display battery icon and level on the upper right corner of the screen.
  // the battery icon has less granularity than the % displayed, which has
  // less granularity than the actual battery %.
  if (voltage_percentage > 95)
  {
    voltage_percentage = 100;
    show_bat_level_100();
  }
  else if (voltage_percentage <= 95 && voltage_percentage > 90)
  {
    voltage_percentage = 95;
    show_bat_level_100();
  }
  else if (voltage_percentage <= 90 && voltage_percentage > 85)
  {
    voltage_percentage = 90;
		show_bat_level_100();
  }
	else if (voltage_percentage <= 85 && voltage_percentage > 75)
  {
    voltage_percentage = 85;
		show_bat_level_85();
  }
  else if (voltage_percentage <= 75 && voltage_percentage > 65)
  {
    voltage_percentage = 75;
    show_bat_level_85();
  }
  else if (voltage_percentage <= 65 && voltage_percentage > 50)
  {
    voltage_percentage = 60;
    show_bat_level_50();
  }
	else if (voltage_percentage <= 50 && voltage_percentage > 40)
  {
    voltage_percentage = 50;
		show_bat_level_50();
  }
  else if (voltage_percentage <= 40 && voltage_percentage > 30)
  {
    voltage_percentage = 40;
    show_bat_level_50();
  }
  else if (voltage_percentage <= 30 && voltage_percentage > 25)
  {
    voltage_percentage = 30;
    show_bat_level_25();
  }
	else if (voltage_percentage <= 25 && voltage_percentage > 15)
  {
    voltage_percentage = 20;
		show_bat_level_25();
  }
	else
  {
    voltage_percentage = 15;
		show_bat_level_15();
  }
}

