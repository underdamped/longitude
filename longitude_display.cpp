/*
 * Longitude display functions
 *
 * Moises Beato
 * March 2017
 */
#include "longitude.h"
#include <ILI9341_t3.h>
#include <font_TimesNewRomanItalic.h>
#include <font_Arial.h>
#include <font_LiberationSans.h>

// For optimized ILI9341_t3 library
#define TFT_DC      20
#define TFT_CS      15
#define TFT_RST    255  // 255 = unused, connect to 3.3V
#define TFT_MOSI    11
#define TFT_SCLK    13
#define TFT_MISO    12

ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);

static void show_splash_screen(void);
static void show_idle_screen(void);
static void show_laser_on_screen(void);
static void show_measure_screen(void);

void display_setup(void)
{
    // initiate Display
    tft.begin();
    tft.setRotation(3); //SPI connectors facing left
}

// what we show on the screen depends on our state
void update_display(void)
{
    switch(state)
    {
        case STATE_INIT:
            show_splash_screen();
            delay(3000); // let them bask in the splashscreen glory
            update_bat_level();
            show_bat_percent();
            break;

        case STATE_IDLE:
            show_idle_screen();
            update_bat_level();
            show_bat_percent();
            break;

        case STATE_LASERS_ON:
            show_laser_on_screen();
            update_bat_level();
            show_bat_percent();
            break;

        case STATE_MEASURE:
            show_measure_screen();
            update_bat_level();
            show_bat_percent();
            break;
        case WAIT_IDLE:
            show_measure_screen();
            update_bat_level();
            show_bat_percent();
            break;
        default:
            show_idle_screen();
            update_bat_level();
            show_bat_percent();
    }
}

static void show_splash_screen(void)
{
  //Display underlined time name atop the screen  
  tft.setFont(Arial_14);
  tft.fillScreen(ILI9341_BLACK);  
  tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
  tft.setCursor(50,4);
  tft.println("Divide by Zero Electronics");
  tft.drawFastHLine(2,20,328,ILI9341_RED);
  
  //device name at center
  tft.fillTriangle(100,40,10,149,60,149,ILI9341_RED); //tilted triangle to simulate the pole of the letter "L"
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
  tft.setFont(TimesNewRoman_40_Italic);
  tft.fillRoundRect(230,130,80,20,10,ILI9341_RED);
  tft.fillRect(40,130,200,20,ILI9341_RED);  //red underline 
  tft.setCursor(80,90);
  tft.println("ongitude");

  // Tagline
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setFont(Arial_14); 
  tft.setCursor(70,190);
  tft.println("Making Measurement");
  tft.setCursor(95,210);
  tft.println("a Simple Matter"); 
    return;
}

static void show_idle_screen(void)
{
  // this screen should include the result of the last measurement, if any.
  // (the variable 'measured_length', which stores the result, has scope here)

  //Display underlined time name atop the screen  
  tft.setFont(Arial_14);
  tft.fillScreen(ILI9341_BLACK);  
  tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
  tft.setCursor(50,4);
  tft.println("Divide by Zero Electronics");
  tft.drawFastHLine(2,20,328,ILI9341_RED); //red underline
  
  //show last measurement
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setFont(LiberationSans_16); 
  tft.setCursor(20,50);
  tft.println("Last Measurement:");
  tft.drawRect(10,40,200,65,ILI9341_WHITE); // white rectangle
  tft.setCursor(160,80);
  tft.println( data[unit].id ); 
  tft.setFont(LiberationSans_28); 
  tft.setCursor(40,70); 
  tft.println( data[unit].convert(measured_length) );
  
  //show laser state = OFF
  tft.setFont(LiberationSans_18); 
  tft.setCursor(20,130);
  tft.println("Laser State:  OFF");
  tft.drawRect(10,122,240,30,ILI9341_WHITE); //white rectangle
  
  //show instructions;
  tft.setFont(Arial_14);
  tft.setCursor(10,190);
  tft.println("Click once to turn lasers ON");

  //show mode change  
  tft.setCursor(10,210);
  tft.println("Press Mode for Range Finder");    
    return;   
}

static void show_laser_on_screen(void)
{
  //show laser state = ON
  tft.setFont(LiberationSans_18); 
  tft.setCursor(20,130);
  tft.fillRect(10,122,240,30,ILI9341_BLACK); //black block to clear pervious message
  tft.println("Laser State:  ON ");
  tft.drawRect(10,122,240,30,ILI9341_WHITE); // white rectangle  
  
  //show instructions;
  tft.setFont(Arial_14);
  tft.fillRect(10,180,310,100,ILI9341_BLACK); //black block to clear pervious message
  tft.setCursor(10,190);
  tft.println("Click once to take a measurement");
  
  //show mode change  
  tft.setCursor(10,210);
  tft.println("Press Mode to zero laser");
    return;
}

static void show_measure_screen(void)
{
  // this screen should show the result of the last measurement and
  // put the processor to sleep for a second or so. after the
  // processor wakes up, it will be in STATE_IDLE.
  // Display underlined time name atop the screen  
  tft.setFont(Arial_14);
  tft.fillScreen(ILI9341_BLACK);  
  tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
  tft.setCursor(50,4);
  tft.println("Divide by Zero Electronics");
  tft.drawFastHLine(2,20,328,ILI9341_RED);

  // display length calculation
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setFont(LiberationSans_28);
  tft.setCursor(20,50);
  tft.println("Length:");
  tft.drawRect(10,30,200,110,ILI9341_WHITE);
  tft.setCursor(40,90);
  tft.printf( "%0.3f", data[unit].convert(measured_length) );
  tft.setFont(LiberationSans_20);
  tft.setCursor(180,100);
  tft.println( data[unit].id );

  // Display individual lasers and angle
  tft.setFont(Arial_14);
  tft.setCursor(100,150);
  tft.println("Angle: ");
  tft.setCursor(160,150);
  tft.print(angle);
  tft.setCursor(10,180);
  tft.println("Laser 1: ");
  tft.setCursor(90,180);
  tft.print( data[unit].convert(laser_left.last_measurement), 3 );
  tft.setCursor(170,180);
  tft.println("Laser 2: ");
  tft.setCursor(245,180);
  tft.print( data[unit].convert(laser_right.last_measurement), 3 );

   //show mode change  
  tft.setFont(Arial_14);
  tft.setCursor(10,210);
  tft.println("Press Mode to change units");
  
  return;
}

// display battery icon and percentage
void show_bat_percent(void)
{ 
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setFont(Arial_14);
  tft.fillRect(267,49,40,20,ILI9341_BLACK); // clear previous percentage
  tft.setCursor(268,50);
  tft.print(voltage_percentage);
  tft.setCursor(300,50);
  tft.print("%");  
}
void show_bat_level_100(void){
  tft.fillRoundRect(247,42,16,26,3,ILI9341_BLACK);
  tft.drawRoundRect(245,40,20,30,5,ILI9341_WHITE);
  tft.fillRoundRect(247,42,16,26,3,ILI9341_GREEN);
}

void show_bat_level_85(void){
  tft.fillRoundRect(247,42,16,26,3,ILI9341_BLACK);
  tft.drawRoundRect(245,40,20,30,5,ILI9341_WHITE);
  tft.fillRoundRect(247,50,16,18,3,ILI9341_GREEN);
}

void show_bat_level_50(void){
  tft.fillRoundRect(247,42,16,26,3,ILI9341_BLACK);
  tft.drawRoundRect(245,40,20,30,5,ILI9341_WHITE);
  tft.fillRoundRect(247,55,16,13,3,ILI9341_GREEN);
}

void show_bat_level_25(void){
  tft.fillRoundRect(247,42,16,26,3,ILI9341_BLACK);
  tft.drawRoundRect(245,40,20,30,5,ILI9341_WHITE);
  tft.fillRoundRect(247,60,16,8,3,ILI9341_YELLOW);
}

void show_bat_level_15(void){
  tft.fillRoundRect(247,42,16,26,3,ILI9341_BLACK);
  tft.drawRoundRect(245,40,20,30,5,ILI9341_WHITE);
  tft.fillRoundRect(247,65,16,3,3,ILI9341_RED);

}

