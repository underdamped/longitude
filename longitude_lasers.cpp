/*
 * Longitude laser routines
 * 
 * Javier Lombillo
 * February 2017
 */
#include "longitude.h"

// laser command strings
#define LASER_ON      "$0003260130&"
#define LASER_OFF     "$00022123&"
#define LASER_MEASURE "$00022123&"

// initialize laser data objects
void laser_setup(struct laser *left, struct laser *right)
{
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

void laser_on(struct laser *laser)
{
    laser->port->print( LASER_ON );
    laser->enabled = true;
}

void laser_off(struct laser *laser)
{
    laser->port->print( LASER_OFF );
    laser->enabled = false;
}

void laser_measure(struct laser *laser)
{
    String retcode;
    String distance;

    laser->port->print( LASER_MEASURE );

    retcode = laser->port->readStringUntil( '&' );

    if ( retcode.substring(0,7) == "$000621" )
    {
        distance = retcode.remove(0,7);
        int d = distance.toInt();
        double meters = d / 100000.0;
        
        laser->last_measurement = meters;
    }
}
