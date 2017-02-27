/*
 * Longitude display functions
 *
 * Moises Beato
 * February 2017
 */
#include "longitude.h"
#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"
#include "gfxfont.h"
#include <Fonts/FreeMonoOblique9pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>
#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/FreeSans12pt7b.h>


// TFT setup
#define TFT_CS 15
#define TFT_DC 20
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC);

static struct laser laser_left;
static struct laser laser_right;

static void show_splash_screen(void);
static void show_idle_screen(void);
static void show_laser_on_screen(void);
static void show_measure_screen(void);

void display_setup(void)
{
      
  tft.begin(HX8357D); // initiate Display
  tft.setRotation(3); //header cables facing left
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
            update_bat_level();
            break;

        case STATE_LASERS_ON:
            show_laser_on_screen();
            update_bat_level();
            break;

        case STATE_MEASURE:
            show_measure_screen();
            update_bat_level();
            break;

        default:
            show_idle_screen();
            update_bat_level();
    }
}

// stubs for moises to do his magic
static void show_splash_screen(void)
{
  //print the team name and draws a red underline
  tft.setFont(); //default font
  tft.fillScreen(HX8357_BLACK);  //clear screen
  tft.setTextColor(HX8357_RED, HX8357_BLACK);
  tft.setTextSize(2);
  tft.setCursor(80,5);
  tft.println("Divide by Zero Electronics");
  tft.drawFastHLine(2,30,510,HX8357_RED);
  
  //device name at center
  tft.fillTriangle(145,65,20,179,80,179,HX8357_RED); //tilted triangle to simulate the pole of the letter "L"
  tft.setTextColor(HX8357_YELLOW, HX8357_BLACK);
  tft.setFont(&FreeSerifItalic9pt7b);
  tft.setTextSize(4);
  tft.fillRoundRect(320,160,130,20,10,HX8357_RED);
  tft.fillRect(80,160,250,20,HX8357_RED);  //red underline 
  tft.setCursor(125,155);
  tft.println("ongitude");


  // Tagline
  tft.setTextColor(HX8357_WHITE, HX8357_BLACK);
  tft.setFont(&FreeMonoOblique9pt7b); 
  tft.setTextSize(2);
  tft.setCursor(45,240);
  tft.println("Making Measurement");
  tft.setCursor(75,270);
  tft.println("a Simple Matter"); 
    return;
}

static void show_idle_screen(void)
{
    // this screen should include the result of the last measurement, if any.
    // (the variable 'measured_length', which stores the result, has scope here)
  
  //print the team name and draws a red underline
  tft.setFont();  // sets default font
  tft.fillScreen(HX8357_BLACK);  // clear screen
  tft.setTextColor(HX8357_RED, HX8357_BLACK);
  tft.setTextSize(2);
  tft.setCursor(80,5);
  tft.println("Divide by Zero Electronics");
  tft.drawFastHLine(2,30,510,HX8357_RED);
  
  //show last measurement
  tft.setTextColor(HX8357_WHITE, HX8357_BLACK);
  tft.setFont(&FreeMono12pt7b);
  tft.setTextSize(1);
  tft.setCursor(30,70);
  tft.println("Last Measurement:");
  tft.drawRect(10,50,340,100,HX8357_WHITE); 
  tft.setTextSize(2);
  tft.setCursor(60,120);
  tft.println(measured_length);
  tft.setTextSize(1);
  tft.setCursor(190,120);
  tft.println("Meter(s)");
  
  //show laser state = OFF
  tft.setCursor(20,180);
  tft.println("Laser State:  OFF");
  tft.drawRect(10,160,280,30,HX8357_WHITE);  
  
  //show instructions
  tft.setFont(&FreeMonoOblique9pt7b);
  tft.setTextSize(1);
  tft.setCursor(50,250);
  tft.println("Click Once to turn");
  tft.setCursor(90,270);
  tft.println("lasers ON");
    return;
}

static void show_laser_on_screen(void)
{
  //print the team name and draws a red underline
  tft.setFont();
  tft.fillScreen(HX8357_BLACK);  
  tft.setTextColor(HX8357_RED, HX8357_BLACK);
  tft.setTextSize(2);
  tft.setCursor(80,5);
  tft.println("Divide by Zero Electronics");
  tft.drawFastHLine(2,30,510,HX8357_RED);
  
  //show last measurement
  tft.setTextColor(HX8357_WHITE, HX8357_BLACK);
  tft.setFont(&FreeMono12pt7b);
  tft.setTextSize(1);
  tft.setCursor(30,70);
  tft.println("Last Measurement:");
  tft.drawRect(10,50,340,100,HX8357_WHITE); 
  tft.setTextSize(2);
  tft.setCursor(60,120);
  tft.println(measured_length);
  tft.setTextSize(1);
  tft.setCursor(190,120);
  tft.println("Meter(s)");
  
  //show laser state = ON
  tft.setTextColor(HX8357_WHITE, HX8357_BLACK);
  tft.setFont(&FreeMono12pt7b);
  tft.setTextSize(1);
  tft.setCursor(20,180);
  tft.println("Laser State:  ON ");
  tft.drawRect(10,160,280,30,HX8357_WHITE); 
  
  //show instructions
  tft.setFont(&FreeMonoOblique9pt7b);
  tft.setTextSize(1);
  tft.setCursor(50,250);
  tft.println("Click Once to take ");
  tft.setCursor(70,270);
  tft.println("a measurement   ");
    return;
}

static void show_measure_screen(void)
{
    // this screen should show the result of the last measurement and
    // put the processor to sleep for a second or so. after the
    // processor wakes up, it will be in STATE_IDLE.

    //Display underlined time name atop the screen  
  tft.setFont();
  tft.fillScreen(HX8357_BLACK);  
  tft.setTextColor(HX8357_RED, HX8357_BLACK);
  tft.setTextSize(2);
  tft.setCursor(80,5);
  tft.println("Divide by Zero Electronics");
  tft.drawFastHLine(2,30,510,HX8357_RED);

 tft.setTextColor(HX8357_WHITE, HX8357_BLACK);
  tft.setFont(&FreeSans12pt7b);
  tft.setTextSize(2);
  tft.setCursor(130,115);
  tft.println("Length:");
  tft.drawRect(60,70,310,150,HX8357_WHITE);
  tft.setTextSize(2);
  tft.setCursor(100,170);
  tft.print(measured_length,3);
  tft.setTextSize(1);
  tft.setCursor(230,200);
  tft.println("Meter(s)");

//for debuggin
  double angle = get_angle();
  tft.setFont();
  tft.setTextSize(2);
  tft.setCursor(140,250);
  tft.println("Angle: ");
  tft.setCursor(220,250);
  tft.print(angle);
  tft.setCursor(20,280);
  tft.println("Laser 1: ");
  tft.setCursor(120,280);
  tft.print(laser_right.last_measurement,3);
  tft.setCursor(250,280);
  tft.println("Laser 2: ");
  tft.setCursor(350,280);
  tft.print(laser_left.last_measurement,3);  
    return;
}
// battery level functions
void show_bat_level_100(void){
  tft.setFont(); // default font
  tft.fillRoundRect(392,42,16,26,3,HX8357_BLACK);
  tft.drawRoundRect(390,40,20,30,5,HX8357_WHITE);
  tft.fillRoundRect(392,42,16,26,3,HX8357_GREEN);
}

void show_bat_level_75(void){
  tft.setFont(); // default font
  tft.fillRoundRect(392,42,16,26,3,HX8357_BLACK);
  tft.drawRoundRect(390,40,20,30,5,HX8357_WHITE);
  tft.fillRoundRect(392,50,16,18,3,HX8357_GREEN);
}

void show_bat_level_50(void){
  tft.setFont(); // default font
  tft.fillRoundRect(392,42,16,26,3,HX8357_BLACK);
  tft.drawRoundRect(390,40,20,30,5,HX8357_WHITE);
  tft.fillRoundRect(392,55,16,13,3,HX8357_GREEN);
}

void show_bat_level_25(void){
  tft.setFont(); // default font
  tft.fillRoundRect(392,42,16,26,3,HX8357_BLACK);
  tft.drawRoundRect(390,40,20,30,5,HX8357_WHITE);
  tft.fillRoundRect(392,60,16,8,3,HX8357_YELLOW);
}

void show_bat_level_15(void){
  tft.setFont(); // default font
  tft.fillRoundRect(392,42,16,26,3,HX8357_BLACK);
  tft.drawRoundRect(390,40,20,30,5,HX8357_WHITE);
  tft.fillRoundRect(392,65,16,3,3,HX8357_RED);

}
void show_bat_percent(void)
{ //show the battery percentage
  get_bat_level();
  tft.setFont();
  tft.setTextSize(2);
  tft.setCursor(420,50);
  tft.printf("%d \%",voltage_percentage);
}

