/*
 * Longitude main loop (state machine)
 * 
 * Javier Lombillo, Moises Beato
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

double measured_length;

void loop()
{
    double angle;
    show_bat_percent();  // real time battery percentage updates
    show_bat_icon();     // update battery icon every 5 seconds
    // program behavior is driven by an FSM
    while (1)
    {
        switch(state)
        {
            case STATE_INIT: // start-up state, where we initialize things

                setup();
                update_display(); // In this state we show splash screen
                state = STATE_IDLE;
                break;

            case STATE_IDLE: // waiting for user input

                update_display();    // in this state we show idle screen
                state = WAIT_LASER_ON;                
                break;
                
            case WAIT_LASER_ON:
                
                if ( b_measure.state == ACTIVE ) 
                {
                    laser_on( &laser_left );
                    laser_on( &laser_right );

                    state = STATE_LASERS_ON;
                    b_measure.state = INACTIVE; // reset button state
                }
                break;

            case STATE_LASERS_ON: // user is aiming the lasers

                update_display(); // we show idle screen + laser on messege 
                state = WAIT_MEASURE;                 
                break;
                
            case WAIT_MEASURE: // spin here until user presses button

                if ( b_measure.state == ACTIVE ) // user wants a measurement
                {
                    // the lasers require time to take a measurement, so we'll send the measure command first
                    laser_measure( &laser_left );
                    laser_measure( &laser_right );

                    // while the lasers are doing their thing, get the sensor angle
                    angle = get_angle();

                    // finally, check the serial buses for measurement data
                    laser_read_data( &laser_left );
                    laser_read_data( &laser_right );

                    measured_length = calc_length( angle, laser_left.last_measurement, laser_right.last_measurement );

                    b_measure.state = INACTIVE;
                    state = STATE_MEASURE;
                }
                break;
                
            case STATE_MEASURE: // show measurement results

                  update_display(); // in this state we show the measured length
                  state = WAIT_IDLE;
                  break;

            case WAIT_IDLE: // spin here until user presses button

                if ( b_measure.state == ACTIVE ) // user wants to move to main screen
                {
                  b_measure.state = INACTIVE;
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
