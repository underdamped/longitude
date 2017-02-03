/*
 * Longitude button handlers
 */
#include "longitude.h"

static bool get_button_state(int);

void check_buttons(void)
{
    int i;

    for ( i = 0; i < TOTAL_BUTTONS; i++ )
        button[i] = get_button_state( i );

}

// code stub for reading from GPIO pins
static bool get_button_state(int butt)
{
}
