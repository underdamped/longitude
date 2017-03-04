/*
 * Longitude laser routines
 * 
 * Javier Lombillo
 * February 2017
 */
#include "longitude.h"
#include "Arduino.h"

// laser command codes
#define LASER_ON      "$0003260130&"
#define LASER_MEASURE "$00022123&"

// laser response codes
#define LASER_REPLY           "$00023335"         // laser is talking
#define LASER_ON_CONFIRM      "$0003260130"       // lights are on
#define LASER_MEASURE_CONFIRM "$00022123"         // measurement coming in
#define LASER_TOO_CLOSE       "$0006210000001542" // object is too close to laser
#define LASER_NO_ECHO         "$0006210000001643" // no reflection received
#define LASER_TOO_STRONG      "$0006210000001744" // laser reflection is too strong
#define LASER_TOO_MUCH_LIGHT  "$0006210000001845" // too much ambient light

// constants for handling the serial bus
#define LASER_REPLY_SIZE 10
#define LASER_ON_CONFIRM_SIZE 12
#define LASER_MEASUREMENT_SIZE 17

// initialize laser data objects
void laser_setup(struct laser *left, struct laser *right)
{
    // for debugging
    //Serial.begin(115200);
    //Serial.printf( "[Longitude] initializing...\n" );
    
    Serial2.begin(115200);
    Serial3.begin(115200);
    
    left->id = 0;
    left->port = &Serial2;
    left->enabled = false;
    left->last_measurement = 0.0;

    right->id = 1;
    right->port = &Serial3;
    right->enabled = false;
    right->last_measurement = 0.0;
}

// turn lasers on and wait for confirmation (TODO: handle errors)
void laser_on(struct laser *laser)
{
    String retcode;
    
    laser->port->print( LASER_ON );

    //Serial.printf( "[LASER %d] (lights on)\n", laser->id );
    
    // wait for laser reply code
    while ( laser->port->available() < LASER_REPLY_SIZE );

    retcode = laser->port->readStringUntil( '&' );
    
    if ( retcode == LASER_REPLY ) // laser is sending response
    {
      // wait for lights-up confirmation
      while ( laser->port->available() < LASER_ON_CONFIRM_SIZE );

      retcode = laser->port->readStringUntil( '&' );

      if ( retcode == LASER_ON_CONFIRM )
      {
        //Serial.printf( "[LASER %d] received LASER_ON_CONFIRM: ", laser->id );
        //Serial.println( retcode );
        laser->enabled = true;
        return;
      }
      else // TODO: handle error codes
      {
        //Serial.printf( "[LASER %d] got error code: ", laser->id );
        //Serial.println( retcode );
        return;
      }
    } // no LASER_REPLY string (noise on the bus?)
    else
    {
      //Serial.printf( "[LASER %d] (lasers on) received unexpected data: ", laser->id );
      //Serial.println( retcode );
    }
}

// send measurement command to lasers; returns without waiting for the result
void laser_measure(struct laser *laser)
{
    //Serial.printf( "[LASER %d] sending measure command\n", laser->id );
    laser->port->print( LASER_MEASURE );

    return;
}

// read the result of a measurement from the appropriate serial bus
void laser_read_data(struct laser *laser)
{
    String retcode;
    String distance;

    // wait for reply code
    while ( laser->port->available() < LASER_REPLY_SIZE );

    retcode = laser->port->readStringUntil( '&' );

    //Serial.printf( "[LASER %d] (read) received reply code: ", laser->id );
    //Serial.println( retcode );
    
    if ( retcode == LASER_REPLY )
    {
      // wait for measurement code
      while ( laser->port->available() < LASER_MEASUREMENT_SIZE );

      retcode = laser->port->readStringUntil( '&' );

      //Serial.printf( "[LASER %d] measurement code: ", laser->id );
      //Serial.println( retcode );

      // parsing errors in the measurement code will be difficult: the laser module
      // designer stupidly uses the same code format for valid and invalid measurements,
      // so the only way to know if the result of a measurement should be interpreted as
      // a distance or an error code is by the length of time it takes to receive the
      // code (errors take "about 5 seconds").  dumb.
      if ( retcode.substring(0,7) == "$000621" ) // measurement code prefix
      {
          distance = retcode.remove(0,7);
          int d = distance.toInt();
          double meters = d / 100000.0;
          
          laser->last_measurement = meters;
          //Serial.printf( "[LASER %d] distance: %d [%0.8f meters]\n", laser->id, d, meters );
          laser->enabled = false;
          return;
      }
      else // this should never happen
      {
        //Serial.printf( "[LASER %d] received reply, but unknown return code: ", laser->id );
        //Serial.println( retcode );
      }
    }
    else // no LASER_REPLY string
    {
      //Serial.printf( "[LASER %d] (measurement) received unexpected data\n", laser->id );
    }
}
