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
struct btn b_UP;
struct btn b_DN;

void ISR_measure(void);
void ISR_mode(void);
void ISR_UP(void);
void ISR_DN(void);

void button_setup(void)
{
  //declare button pins
  b_measure.pin = 5;
  b_mode.pin    = 4;
  b_UP.pin      = 3;
  b_DN.pin      = 2;

  //initialize buttons as inactive 
  b_measure.state = INACTIVE;
  b_mode.state    = INACTIVE;
  b_UP.state      = INACTIVE;
  b_DN.state      = INACTIVE;

  pinMode(b_measure.pin, INPUT_PULLUP);
  pinMode(b_mode.pin, INPUT_PULLUP);
  pinMode(b_UP.pin, INPUT_PULLUP);
  pinMode(b_DN.pin, INPUT_PULLUP);
  
  //every button is an interrupt
  attachInterrupt( b_measure.pin, ISR_measure, LOW );
  attachInterrupt( b_mode.pin, ISR_mode, LOW );
  attachInterrupt( b_UP.pin, ISR_UP, LOW );
  attachInterrupt( b_DN.pin, ISR_DN, LOW );
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
void ISR_UP(void)
{
    cli();
    b_UP.state = ACTIVE;
    sei();    
}
void ISR_DN(void)
{
    cli();
    b_measure.state = ACTIVE;
    sei();    
}
