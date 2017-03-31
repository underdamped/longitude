/*
 * Longitude EEPROM configuration stuff
 * 
 * Javier Lombillo
 * March 2017
 */
#include <EEPROM.h>
#include <string.h>
#include "longitude.h"

#define CONFIG_ADDR_START 0 // base EEPROM address of configuration data

// magic bytes to signify a valid Longitude configuration
static const uint16_t CONFIG_SIGNATURE = 0xBEEF;

// addresses of the configuration variables
static const uint16_t CONFIG_ADDR_ANGLE_OFFSET = CONFIG_ADDR_START        + sizeof CONFIG_SIGNATURE;
static const uint16_t CONFIG_ADDR_UNIT         = CONFIG_ADDR_ANGLE_OFFSET + sizeof angle_offset;

static void store_double_eeprom(uint16_t, double);
static double load_double_eeprom(uint16_t);

// download default configuration from EEPROM
void load_config(void)
{
  // first check if a config actually exists by looking for
  // the CONFIG_SIGNATURE; if it doesn't exist, store the
  // default config
  if ( eeprom_read_word(CONFIG_ADDR_START) != CONFIG_SIGNATURE )
  {
    eeprom_write_word( CONFIG_ADDR_START, CONFIG_SIGNATURE );
    store_double_eeprom( CONFIG_ADDR_ANGLE_OFFSET, angle_offset );
    eeprom_write_word( (uint16_t *)CONFIG_ADDR_UNIT, unit );
  }
  else
  {
    angle_offset = load_double_eeprom( CONFIG_ADDR_ANGLE_OFFSET );
    unit = (UNITS)eeprom_read_word( (uint16_t *)CONFIG_ADDR_UNIT );
  }
}

// write the specified config value to EEPROM
void save_config(const char *var)
{
  if ( !strcmp(var, "angle") )
    store_double_eeprom( CONFIG_ADDR_ANGLE_OFFSET, angle_offset );
  else if ( !strcmp(var, "unit") )
    eeprom_write_word( (uint16_t *)CONFIG_ADDR_UNIT, unit );
}

// print the config for debugging purposes
void print_config(void)
{
  uint16_t sig = 0, u = 0;
  double a = 0.0;
  const char *units[] = { "meters", "feet", "inches" };

  sig = eeprom_read_word( CONFIG_ADDR_START );
  a = load_double_eeprom( CONFIG_ADDR_ANGLE_OFFSET );
  u = eeprom_read_word( (uint16_t *)CONFIG_ADDR_UNIT );
  
  //Serial.printf( "Config [%X]: angle offset: %0.4f, units: %s\n", sig, a, units[u] );
}

// calling this overwrites the magic bytes signature in the EEPROM, forcing a
// default config on the next boot
void clear_config(void)
{
  eeprom_write_word( CONFIG_ADDR_START, 0 );
}

// store a double-precision (64-bit) value in the EEPROM
static void store_double_eeprom(uint16_t addr, double val)
{
  eeprom_write_block( (const void *)&val, (void *)addr, (uint32_t)sizeof(val) );
}

// load a double-precision value from the EEPROM
static double load_double_eeprom(uint16_t addr)
{
  double x;
  
  eeprom_read_block( (void *)&x, (const void *)addr, (uint32_t)sizeof(x) );

  return x;
}
