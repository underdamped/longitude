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
static void zero_angle(void);

// the laser "objects"
struct laser laser_left;
struct laser laser_right;

enum FSM state;

double measured_length;
double angle;

static double angle_offset = 0;

void loop()
{
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
                    angle += angle_offset;

                    // finally, check the serial buses for measurement data
                    laser_read_data( &laser_left );
                    laser_read_data( &laser_right );

                    measured_length = calc_length( angle, laser_left.last_measurement, laser_right.last_measurement );

                    b_measure.state = INACTIVE;
                    state = STATE_MEASURE;
                }

                // pressing the mode button while the lasers are on will zero the angle sensor
                if ( b_mode.state == ACTIVE )
                {
                    zero_angle();
                    b_mode.state = INACTIVE;
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

    // set the internal ADC with a 16-sample moving average
    analogReadAveraging(16);
}

// when the user points the lasers at the ends of an object, there is an
// implicit triangle formed by the two laser dots and the center of the device.
// since the laser modules tell us the distances to the dots -- giving us the
// length of each leg -- and the angle sensor tells us the angle between the
// legs, we can solve for the length between the two laser dots using the law
// of cosines.  since the lasers are physically offset from each other (not
// coincident), we add the distance between them to the derived length.
static double calc_length(double theta, double a, double b)
{
    double phi, len;
    
    // convert degrees to radians
    phi = theta * M_PI/180.0;

    // solve the triangle
    len = sqrt( a*a + b*b - 2*a*b*cos(phi) );

    // add the laser offset
    return (len + LASER_OFFSET);
}

// the idea here is to give the user a way to zero the angle sensor for a more
// precise measurement.  while in the laser-aiming state, pressing the mode button
// calls this function, which takes an angle measurement and saves the offset for
// future calculations
static void zero_angle(void)
{
    angle_offset = 0.0 - get_angle();
}

