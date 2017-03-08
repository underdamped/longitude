/*
 * Longitude header
 * 
 * Javier Lombillo
 * February 2017
 */
#ifndef LONGITUDE_HEADER
#define LONGITUDE_HEADER

#include <HardwareSerial.h>

#define VERSION 0.9

#define LASER_OFFSET 0.05 // distance in meters between the two lasers

// we're using active-low logic for the buttons; these make the code more readable
#define ACTIVE LOW
#define INACTIVE HIGH

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

// button object
struct btn
{
    volatile bool state;   // ACTIVE or INACTIVE
    uint8_t pin;   
};
extern struct btn b_measure; //button to measure and select
extern struct btn b_mode;    //button to switch nmode
extern struct btn b_UP;      
extern struct btn b_DN;      

// measurement display units; we use these to index into the data[] conversion array
extern enum UNITS { meter, foot, inch } unit;

// unit bookkeeping
struct unit_conversion
{
    const char *id;            // unit identifier, e.g., "ft"
    double (*convert)(double); // conversion routine
};
extern struct unit_conversion data[];

// global vars accessible by the display routines
extern double measured_length;
extern uint8_t voltage_percentage;
extern struct laser laser_left;
extern struct laser laser_right;
extern double angle;

// longitude_lasers.c
void laser_setup(struct laser *, struct laser *);
void laser_on(struct laser *);
void laser_measure(struct laser *);
void laser_read_data(struct laser *);

// longitude_adc.c
int adc_setup(void);
double get_angle(void);

// longitude_buttons.c
void button_setup(void);

// longitude_display.c
void display_setup(void);
void update_display(void);
void show_bat_percent(void);
void show_bat_level_100(void);
void show_bat_level_85(void);
void show_bat_level_50(void);
void show_bat_level_25(void);
void show_bat_level_15(void);

// longitude_battery.c
void get_bat_level(void);
void update_bat_level(void); 


#endif
