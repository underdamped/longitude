/*
 * Longitude display functions
 */
#include "longitude.h"

static void show_splash_screen(void);
static void show_idle_screen(void);
static void show_laser_on_screen(void);
static void show_measure_screen(void);

// what we show on the screen depends on our state
void update_display(void)
{
    switch(state)
    {
        case STATE_INIT:
            show_splash_screen();
            break;

        case STATE_IDLE:
            show_idle_screen();
            break;

        case STATE_LASERS_ON:
            show_laser_on_screen();
            break;

        case STATE_MEASURE:
            show_measure_screen();
            break;

        default:
            show_idle_screen();
    }
}

// stubs for moises to do his magic
static void show_splash_screen(void)
{
    return;
}

static void show_idle_screen(void)
{
    // this screen should include the result of the last measurement, if any
    // the variable 'measured_length', which stores the result, has scope here
    return;
}

static void show_laser_on_screen(void)
{
    return;
}

static void show_measure_screen(void)
{
    // this screen should show the result of the last measurement and
    // put the processor to sleep for a second or so. after the
    // processor wakes up, it will be in STATE_IDLE.
    return;
}
