/*
 * Longitude display functions
 *
 * Moises Beato
 * February 2017
 */
#include "longitude.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
//#include "SPI.h"

// TFT setup
#define TFT_DC 20
#define TFT_CS 15
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

static struct laser laser_left;
static struct laser laser_right;

static void show_splash_screen(void);
static void show_idle_screen(void);
static void show_laser_on_screen(void);
static void show_measure_screen(void);

void display_setup(void)
{
      // initiate Display
    tft.begin();
    tft.setRotation(1);
}
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
    //print the team name and draws a red underline
  tft.fillScreen(ILI9341_BLACK);  
  tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(5,5);
  tft.println("Divide by Zero Electronics");
  tft.drawFastHLine(2,30,310,ILI9341_RED);
  
  //device name at center
  tft.fillTriangle(95,60,50,144,80,144,ILI9341_RED); //tilted triangle to simulate the pole of the letter "L"
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.setCursor(100,100);
  tft.println("ongitude");
  tft.fillRect(60,130,200,15,ILI9341_RED);  //red underline 

  // Super-Mega-Special-Awesome Tagline display
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(40,160);
  tft.println("Making Measurement a          Simple Matter"); 
  delay(2500); // appreciate the awesomeness
  tft.fillScreen(ILI9341_BLACK); //now, move along
    return;
}

static void show_idle_screen(void)
{
    // this screen should include the result of the last measurement, if any.
    // (the variable 'measured_length', which stores the result, has scope here)

  //Display underlined time name atop the screen  
  tft.fillScreen(ILI9341_BLACK);  
  tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(5,5);
  tft.println("Divide by Zero Electronics");
  tft.drawFastHLine(2,30,310,ILI9341_RED); //red underline
  
  //show last measurement
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20,50);
  tft.println("Last Measurement:");
  tft.drawRect(10,40,280,60,ILI9341_WHITE); 
  tft.setTextSize(2);
  tft.setCursor(50,70);
  tft.println(measured_length);
  tft.setTextSize(2);
  tft.setCursor(180,70);
  tft.println("Meter(s)");
  //show laser state = OFF
  tft.setCursor(20,130);
  tft.println("Laser State:  OFF");
  tft.drawRect(10,120,280,30,ILI9341_WHITE);   
  //show instructions
  tft.setTextSize(2);
  tft.setCursor(50,200);
  tft.println("Click Once to turn");
  tft.setCursor(90,220);
  tft.println("lasers ON");
    return;
}

static void show_laser_on_screen(void)
{
  //show laser state = ON
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20,130);
  tft.println("Laser State:  ON ");
  tft.drawRect(10,120,280,30,ILI9341_WHITE); 
  //show instructions
  tft.setTextSize(2);
  tft.setCursor(50,200);
  tft.println("Click Once to take ");
  tft.setCursor(70,220);
  tft.println("a measurement   ");
    return;
}

static void show_measure_screen(void)
{
    // this screen should show the result of the last measurement and
    // put the processor to sleep for a second or so. after the
    // processor wakes up, it will be in STATE_IDLE.

    //Display underlined time name atop the screen  
  tft.fillScreen(ILI9341_BLACK);  
  tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(5,5);
  tft.println("Divide by Zero Electronics");
  tft.drawFastHLine(2,30,310,ILI9341_RED); //red underline

  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(4);
  tft.setCursor(80,50);
  tft.println("Length:");
  tft.drawFastHLine(50,90,210,ILI9341_RED);

  tft.setTextSize(3);
  tft.setCursor(50,150);
  tft.print(measured_length,5);
  tft.setTextSize(2);
  tft.setCursor(180,180);
  tft.println("Meter(s)");

//for debuggin
double angle = get_angle();
  tft.setTextSize(2);
  tft.setCursor(20,210);
  tft.println("Angle: ");
  tft.setCursor(120,210);
  tft.print(angle);  
    return;
}
