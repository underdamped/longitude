/*
 * Longitude main loop (state machine)
 * 
 * Javier Lombillo, Moises Beato
 * February 2017
 */
#include <math.h>
#include "longitude.h"

// local routines
static void zero_angle(void);
static double calc_length(double theta, double a, double b);
static double to_meter(double);
static double to_feet(double);
static double to_inch(double);

// the laser "objects"
struct laser laser_left;
struct laser laser_right;

// array of unit conversion stuff, indexed by 'unit' enum
struct unit_conversion data[] = {
    { "m",  &to_meter },
    { "ft", &to_feet },
    { "in", &to_inch },
};

enum FSM state;
enum UNITS unit;

double measured_length;
double angle;

static double angle_offset = 0;

void loop()
{
    // program behavior is driven by an FSM
    switch(state)
    {
        case STATE_INIT: // start-up state, where we initialize things

            setup();
            update_display(); // show splash screen
            state = STATE_IDLE;
            break;

        case STATE_IDLE: // waiting for user input

            update_display(); // show idle screen
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
            
        case STATE_MEASURE:

            update_display(); // show the measured length
            state = WAIT_IDLE;
            break;

        case WAIT_IDLE: // spin here until user presses button

            if ( b_measure.state == ACTIVE ) // user wants to move to main screen
            {
              b_measure.state = INACTIVE;
              state = STATE_IDLE;
            }
            // pressing mode button while the measurement is showing changes units.
            if ( b_mode.state == ACTIVE )
            {
                unit = (UNITS)((unit + 1) % 3);
                update_display();
                b_mode.state = INACTIVE;
            }
            break;

        default: // should never happen, but go to known state if we're totally hosed
            state = STATE_INIT;
    }
}

void setup()
{
    adc_setup();
    button_setup();
    display_setup();
    laser_setup( &laser_left, &laser_right );

    // set the internal ADC to use a 32-sample moving average
    analogReadAveraging(32);

    // set default units display ('meter', 'foot', or 'inch')
    unit = meter;
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
    phi = theta * M_PI / 180.0;

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

// unit conversion routines; each is passed a double-precision value in units of meters
double to_meter(double m)
{
    return m;
}

double to_feet(double m)
{
    return (m * 3.280839895013123359L);
}

double to_inch(double m)
{
    return (m * 39.37007874015748031L);
}

