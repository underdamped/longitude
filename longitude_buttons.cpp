/*
 * Longitude button handlers
 */
#include "longitude.h"
#include "Arduino.h"

#define button_1 5                  
#define button_2 6
#define button_3 23
#define button_4 22

static bool get_button_state(int);
void button_setup(void)
{
 pinMode(button_1, INPUT);
 pinMode(button_2, INPUT);
 pinMode(button_3, INPUT);
 pinMode(button_4, INPUT);
}
void check_buttons(void)
{
    int i;

    for ( i = 0; i < TOTAL_BUTTONS; i++ )
        button[i] = get_button_state( i );

}

// code stub for reading from GPIO pins
static bool get_button_state(int butt)
{
   int button_state;
   switch(butt)
   {
    case 0:
      button_state = digitalRead(button_1);
      break;
    case 1:
      button_state = digitalRead(button_2);
      break;
    case 2:
      button_state = digitalRead(button_3);
      break;
    case 3:
      button_state = digitalRead(button_4);
      break;
    default:
      break;            
   }
    delay(50);
    return button_state;
}
