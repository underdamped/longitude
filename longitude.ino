/*
 * Longitude main loop (state machine)
 * 
 * Javier Lombillo
 * February 2017
 */
#include <math.h>
#include "longitude.h"


// local routines
static double calc_length(double theta, double a, double b);

// the laser "objects"
static struct laser laser_left;
static struct laser laser_right;

enum FSM state;

void button_setup(void);

bool button[TOTAL_BUTTONS];
double measured_length;

void loop()
{
    double angle;

    // program behavior is driven by an FSM
    while (1)
    {
        switch(state)
        {
            case STATE_INIT: // start-up state, where we initialize things

                setup();
                update_display();
                state = STATE_IDLE;
                break;

            case STATE_IDLE: // waiting for user input

                update_display();       
                state = WAIT_LASER_ON;                
                break;
                
            case WAIT_LASER_ON:
                check_buttons();
                if ( button[0] == false ) // button[0] is the "measure" button
                {
                    laser_on( &laser_left );
                    laser_on( &laser_right );

                    state = STATE_LASERS_ON;
                }
                break;
            case STATE_LASERS_ON: // user is aiming the lasers
                
                update_display();
                state = WAIT_MEASURE;                 
                break;
                
            case WAIT_MEASURE:

                check_buttons();                

                if ( button[0] == false ) // user wants a measurement
                {
                    angle = get_angle();

                    laser_measure( &laser_left );
                    laser_measure( &laser_right );

                    measured_length = calc_length( angle, laser_left.last_measurement, laser_right.last_measurement );

                    state = STATE_MEASURE;
                }
                break;
                
            case STATE_MEASURE: // show measurement results
                  update_display();
                  state = WAIT_IDLE;
                  break;

            case WAIT_IDLE:
                check_buttons();
                if ( button[0] == false ) // user wants to move to main screen
                {
                  state = STATE_IDLE;
                }
                break;

            default: // should never happen, but go to known state if we're totally hosed
                state = STATE_INIT;
        }
    }
}

void setup()
{
    int i;

    // for debugging
    Serial.begin(115200);


    // initialize buttons to inactive/true state
    for ( i = 0; i < TOTAL_BUTTONS; i++ )
      button[i] = true;

    adc_setup();
    button_setup();
    display_setup();
    laser_setup( &laser_left, &laser_right );
}
    
static double calc_length(double theta, double a, double b)
{
    // convert degrees to radians
    double phi = theta * M_PI/180.0;
    return sqrt( a*a + b*b - 2*a*b*cos(phi) );
}
