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

  // enable the internal pull-up resistors
  pinMode(b_measure.pin, INPUT_PULLUP);
  pinMode(b_mode.pin, INPUT_PULLUP);
  
  // interrupt on button press
  attachInterrupt( b_measure.pin, ISR_measure, ACTIVE );
  attachInterrupt( b_mode.pin, ISR_mode, ACTIVE );
}

void ISR_measure(void)
{
    static unsigned long lastIRQ = 0;
    unsigned long currentIRQ = millis();

    // simple, effective software debounce
    if ( (currentIRQ - lastIRQ) > 20 )
        b_measure.state = ACTIVE;

    lastIRQ = currentIRQ;
}

void ISR_mode(void)
{
    static unsigned long lastIRQ = 0;
    unsigned long currentIRQ = millis();

    if ( (currentIRQ - lastIRQ) > 20 )
        b_mode.state = ACTIVE;

    lastIRQ = currentIRQ;
}
