/*
 * Longitude main loop (state machine)
 * 
 * Javier Lombillo, Moises Beato
 * February 2017
 */
#include <math.h>
#include "longitude.h"

#define BEEP_PIN 3 // speaker output pin

enum BEEPS { booting, finished, mode_change, special, beethoven, charge };

// local state variable (EEPROM config)
static bool unit_changed = false;

// local routines
static double calc_length(double theta, double a, double b);
static double to_meter(double);
static double to_feet(double);
static double to_inch(double);
static void beep(BEEPS);

/* globals */
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
double angle_offset;
double angle;

void loop()
{
    // program behavior is driven by an FSM
    switch(state)
    {
        case STATE_INIT:

            beep( booting );
            update_display(); // shows splash screen
            state = STATE_IDLE;
            break;

        case STATE_IDLE:

            update_display(); // shows idle screen

            if ( unit_changed )
            {
              save_config( "unit" );
              unit_changed = false;
            }

            state = WAIT_LASER_ON;                
            break;
            
        case WAIT_LASER_ON: // spin here until user presses a button
            
            if ( b_measure.state == ACTIVE ) // user wants a measurement, light up the fires
            {
                beep( mode_change );
                laser_on( &laser_left );
                laser_on( &laser_right );

                state = STATE_LASERS_ON;
                b_measure.state = INACTIVE;
            }
            else if ( b_mode.state == ACTIVE ) // rangefinder mode
            {
                beep( special );
                laser_on( &laser_left );

                b_mode.state = INACTIVE;
                state = STATE_ONE_LASER;
                single_laser_message();
            }

            break;

        case STATE_ONE_LASER: // user is aiming a single laser

             if ( b_measure.state == ACTIVE )
             {
                  laser_measure( &laser_left );
                  laser_read_data( &laser_left );

                  beep( finished );

                  measured_length = laser_left.last_measurement + RANGE_OFFSET;

                  b_measure.state = INACTIVE;
                  state = STATE_MEASURE;
             }

             break;

        case STATE_LASERS_ON: // user is aiming the lasers

            update_display(); // we show idle screen + laser on messege 
            state = WAIT_MEASURE;                 
            break;
            
        case WAIT_MEASURE: // spin here until user presses the red button

            if ( b_measure.state == ACTIVE ) // user wants a measurement
            {
                // take the sensitive angle measurement while the system is quiescent
                get_angle();

                // the lasers require time to take a measurement, so we'll send the measure command first (these return quickly)
                laser_measure( &laser_left );
                laser_measure( &laser_right );

                beep( mode_change );

                // check the serial buses for measurement data (these block until the data arrives)
                laser_read_data( &laser_left );
                laser_read_data( &laser_right );

                beep( finished );

                measured_length = calc_length( angle, laser_left.last_measurement, laser_right.last_measurement );

                b_measure.state = INACTIVE;
                state = STATE_MEASURE;
            }
            else if ( b_mode.state == ACTIVE ) // pressing the mode button while the lasers are on will zero the angle sensor
            {
                zero_angle();
                beep( special );

                // store the new offset in the EEPROM
                save_config( "angle" );

                b_mode.state = INACTIVE;
            }
            
            break;
            
        case STATE_MEASURE:

            update_display(); // show the measured length
            state = WAIT_IDLE;

            break;

        case WAIT_IDLE: // spin here until user presses the red button

            if ( b_measure.state == ACTIVE ) // user wants to move to main screen
            {
                beep( mode_change );
                b_measure.state = INACTIVE;
                state = STATE_IDLE;
            }
            else if ( b_mode.state == ACTIVE ) // user wants to change units
            {
                beep( special );
                unit = (UNITS)((unit + 1) % 3);
                update_display();

                unit_changed = true; // we'll write the new units to the EEPROM in the IDLE state
                b_mode.state = INACTIVE;
            }
            break;

        default: // should never happen, but go to known state if we're totally hosed
            state = STATE_INIT;
            break;
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

    // set defaults (unit and angle_offset will be overwritten by config, if available)
    unit = meter; // 'meter', 'foot', or 'inch'
    angle_offset = 0.0;
    measured_length = 0.0;
    state = STATE_INIT;
    
    // download config values from EEPROM
    load_config();
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

    // convert degrees to radians (where pi/180 = 0.0174...)
    phi = theta * 0.017453292519943295L;

    // solve the triangle
    len = sqrt( a*a + b*b - 2*a*b*cos(phi) );

    len += LASER_OFFSET;

    /* system error compensation
     * 
     * the minimum length we can measure is equal to the LASER_OFFSET, which is
     * presently 6 cm.  after careful measurement, we determined that the system
     * error is a constant 6.3 cm for lengths greater than 31 cm.  for lengths
     * between 7 cm and 31 cm, we performed a linear regression on the data, giving
     * us a function y = f(x), mapping the true length x to Longitude's notion of
     * the length y.  taking the inverse of this function gave us the function below.
     */
    if ( (len >= 0.07) && (len < 0.31) )
    {
        len = 1.065L * len + 0.00324L;
    }
    else if ( len >= 0.31 ) // for larger lengths, system error is a constant 6.3 cm
    {
        len += 0.063L; 
    }
    
    return len;
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

// make some noise
static void beep(BEEPS action)
{
  switch (action)
  {
    case booting: // g, d, a, b
      tone( BEEP_PIN, 196, 250 );
      delay( 300 );
      tone( BEEP_PIN, 294, 250 );
      delay( 300 );
      tone( BEEP_PIN, 440, 1000 );
      delay( 600 );
      tone( BEEP_PIN, 494, 1000 );
      break;
      
    case charge: // charge! fanfare
      tone( BEEP_PIN, 196, 100 );
      delay( 150 );
      tone( BEEP_PIN, 262, 100 );
      delay( 150 );
      tone( BEEP_PIN, 330, 100 );
      delay( 150 );
      tone( BEEP_PIN, 392, 300 );
      delay( 300 );
      tone( BEEP_PIN, 330, 50 );
      delay( 150 );
      tone( BEEP_PIN, 392, 500 );
      delay( 500 );
      tone( BEEP_PIN, 523, 500 );
      break;

    case beethoven: // beethoven's 5th
      tone( BEEP_PIN, 311, 100 );
      delay( 200 );
      tone( BEEP_PIN, 311, 100 );
      delay( 200 );
      tone( BEEP_PIN, 311, 100 );
      delay( 200 );
      tone( BEEP_PIN, 262, 1000 );
      delay( 1000 );
      tone( BEEP_PIN, 294, 150 );
      delay( 200 );
      tone( BEEP_PIN, 294, 150 );
      delay( 200 );
      tone( BEEP_PIN, 294, 150 );
      delay( 200 );
      tone( BEEP_PIN, 247, 1250 );
      break;
      
    case finished: // a short trill
      tone( BEEP_PIN, 2000, 50 );
      delay( 50 );
      tone( BEEP_PIN, 3000, 150 );
      delay( 50 );
      tone( BEEP_PIN, 1000, 50 );
      break;
      
    case mode_change: // quick beep
      tone( BEEP_PIN, 5500, 10 );
      break;

    case special: // a grunt
      tone( BEEP_PIN, 300, 10 );
      break;
      
    default:
      return;
  }
}

