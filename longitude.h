/*
 * Longitude header
 * 
 * Javier Lombillo
 * February 2017
 */
#ifndef LONGITUDE_HEADER
#define LONGITUDE_HEADER

#include <HardwareSerial.h>

#define VERSION 0.1
#define TOTAL_BUTTONS 4

// FSM states
extern enum FSM { STATE_INIT, STATE_IDLE, WAIT_LASER_ON, STATE_LASERS_ON, WAIT_MEASURE, STATE_MEASURE, WAIT_IDLE } state;

// laser data object
struct laser
{
    uint8_t id;           // identify the laser (0 is left, 1 is right)
    HardwareSerial *port; // serial port associated with laser
    bool enabled;         // on/off status
    double last_measurement;
};

// holds state of each button
extern bool button[TOTAL_BUTTONS];

// global so display routines can see it
extern double measured_length;


// longitude_lasers.c
void laser_setup(struct laser *, struct laser *);
void laser_on(struct laser *);
void laser_off(struct laser *);
void laser_measure(struct laser *);

// longitude_adc.c
int adc_setup(void);
double get_angle(void);

// longitude_buttons.c
void button_setup(void);
void check_buttons(void);

// longitude_display.c
void display_setup(void);
void update_display(void);

#endif
