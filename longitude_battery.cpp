/*
 * Longitude battery level functions
 *
 * Moises Beato, Javier Lombillo
 * February 2017
 */
#include "longitude.h"
#include "Arduino.h"

// we measure battery level using the internal Teensy ADC at 10-bit resolution (vref = 3.3V).
// the nominal 6V battery voltage is dropped to 3V before the ADC pin, which corresponds to
// an ADC count of 931.  the device will continue running down to VCC = 3.3V, below which the
// display stops working. so 3.3V is our minimum battery voltage, corresponding to 512 ADC counts.
#define BATTERY_MAX    (931)
#define BATTERY_MIN    (512)

uint8_t voltage_percentage;

static void get_bat_level(void)
{
  uint16_t bat_read = analogRead(bat_pin); // analog pins are input by default, so no need to use pinMode

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
    show_bat_level_75();
  }
  else if (voltage_percentage <= 75 && voltage_percentage > 70)
  {
    voltage_percentage = 75;
    show_bat_level_75();
  }
  else if (voltage_percentage <= 70 && voltage_percentage > 65)
  {
    voltage_percentage = 70;
    show_bat_level_75();
  }
  else if (voltage_percentage <= 65 && voltage_percentage > 60)
  {
    voltage_percentage = 65;
    show_bat_level_50();
  }
  else if (voltage_percentage <= 60 && voltage_percentage > 55)
  {
    voltage_percentage = 60;
    show_bat_level_50();
  }
  else if (voltage_percentage <= 55 && voltage_percentage > 50)
  {
    voltage_percentage = 55;
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

