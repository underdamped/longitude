/*
 * Longitude main loop (state machine)
 * 
 * Javier Lombillo, Moises Beato
 * February 2017
 */
#include <math.h>
#include "longitude.h"

#define BEEP_PIN 3 // speaker output pin

enum BEEPS { booting, finished, mode_change, special };

// local state variable (EEPROM config)
static bool unit_changed = false;

// local routines
static void zero_angle(void);
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
double angle;
double angle_offset;

void loop()
{
    // program behavior is driven by an FSM
    switch(state)
    {
        case STATE_INIT: // start-up state, where we initialize things

            setup();
            beep( booting );
            update_display(); // show splash screen
            state = STATE_IDLE;
            break;

        case STATE_IDLE: // waiting for user input

            update_display(); // show idle screen

            if ( unit_changed )
            {
              save_config( "unit" );
              unit_changed = false;
            }

            state = WAIT_LASER_ON;                
            break;
            
        case WAIT_LASER_ON:
            
            if ( b_measure.state == ACTIVE ) 
            {
                beep( mode_change );
                laser_on( &laser_left );
                laser_on( &laser_right );

                state = STATE_LASERS_ON;
                b_measure.state = INACTIVE; // reset button state
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

                  measured_length = laser_left.last_measurement;

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
                // the lasers require time to take a measurement, so we'll send the measure command first (these return quickly)
                laser_measure( &laser_left );
                laser_measure( &laser_right );

                beep( mode_change );
                
                // check the serial buses for measurement data (these block)
                laser_read_data( &laser_left );
                laser_read_data( &laser_right );

                // the system is quiescent once the lasers are off, so get the angle
                angle = get_angle();
                angle += angle_offset;
                
                beep( finished );

                measured_length = calc_length( angle, laser_left.last_measurement, laser_right.last_measurement );

                b_measure.state = INACTIVE;
                state = STATE_MEASURE;
            }

            // pressing the mode button while the lasers are on will zero the angle sensor
            if ( b_mode.state == ACTIVE )
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

    // set default angle_offset to 0 (will be overwritten by config if available)
    angle_offset = 0.0;

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

// make some noise
static void beep(BEEPS action)
{
  switch (action)
  {
    case booting: // beethoven's 5th
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

