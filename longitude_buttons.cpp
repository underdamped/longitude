/*
 * Longitude button handlers
 *
 * Moises Beato
 * February 2017
 */
#include "longitude.h"
#include "Arduino.h"

struct btn b_measure;                  
struct btn b_mode;

void ISR_measure(void);
void ISR_mode(void);

void button_setup(void)
{
  // declare button pins
  b_measure.pin = 5;
  b_mode.pin    = 4;

  // initialize buttons as inactive 
  b_measure.state = INACTIVE;
  b_mode.state    = INACTIVE;

  pinMode(b_measure.pin, INPUT_PULLUP);
  pinMode(b_mode.pin, INPUT_PULLUP);
  
  // interrupt on button press (active LOW)
  attachInterrupt( b_measure.pin, ISR_measure, LOW );
  attachInterrupt( b_mode.pin, ISR_mode, LOW );
}

void ISR_measure(void)
{
    cli();
    b_measure.state = ACTIVE;
    sei();    
}
void ISR_mode(void)
{
    cli();
    b_mode.state = ACTIVE;
    sei();    
}
