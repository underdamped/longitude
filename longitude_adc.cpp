/*
 * Longitude MCP3421 ADC driver and angle calculation
 * 
 * Javier Lombillo
 * November 2016
 */

#include <i2c_t3.h> // i2c library
#include "longitude.h"

// I2C address for MCP3421
#define ADC_ADDRESS 0x68    // default address is 0110 1000

// [MCP3421 resolution details]
//
// resolution | data rate | min code | max code | LSB (uV) | measured rate
// -----------------------------------------------------------------------
//  18-bit     3.75 SPS     -131072    131071     15.625       4.15 SPS
//  16-bit       15 SPS     -32768     32767      62.500      16.50 SPS
//  14-bit       60 SPS     -8192      8191      250.000      64.47 SPS
//  12-bit      240 SPS     -2048      2047     1000.000     235.85 SPS

// set to N = {12, 14, 16, 18} for native (non-oversampled) resolution, e.g., 18 for 18-bit
#define RESOLUTION 16

// the MCP3421 is a delta-sigma converter, which uses oversampling internally, so
// there's little benefit in adding another oversampling/decimation layer here.
// there is, however, significant dc-stability advantage in averaging. this comes
// at the cost of reduced throughput (less effective samples-per-second rate).
//
// we implement a simple moving-average filter here; to enable it, set WINDOW_SIZE
// below to the desired window size in samples (1 disables the filter).
#define WINDOW_SIZE 4

// maximum positive code for each resolution
#define ADC_MAX18 0x1FFFF
#define ADC_MAX16 0x7FFF
#define ADC_MAX14 0x1FFF
#define ADC_MAX12 0x7FF

#if RESOLUTION == 18
  #define LSB 0.000015625L  // 15.625 uV
  #define PACKET_SIZE 4     // 3 bytes of data, 1 byte of config
  #define ADC_RES 0x0C      // config bits
  #define ADC_MAX 0x1FFFF   // maximum positive code
#elif RESOLUTION == 16
  #define LSB 0.0000625L    // 62.5 uV
  #define PACKET_SIZE 3     // 2 bytes of data, 1 byte of config
  #define ADC_RES 0x08      // config bits
  #define ADC_MAX 0x7FFF
#elif RESOLUTION == 14
  #define LSB 0.00025L      // 250 uV
  #define PACKET_SIZE 3
  #define ADC_RES 0x04
  #define ADC_MAX 0x1FFF
#else
  #define LSB 0.001L        // 1 mV steps at 12-bits resolution
  #define PACKET_SIZE 3
  #define ADC_RES 0x00
  #define ADC_MAX 0x7FF
#endif

// MCP3421 configuration register
// ------------------------------
// bit #   :  7    | 6   5  | 4    | 3   2  | 1   0
// bit name:  /RDY | C1  C0 | /O,C | S1  S0 | G1  G0
// desc    :  flag | chans  | mode | res    | pga
//
// PGA gain | bits [1:0] values
// ---------------------
//   unity    00
//      2x    01
//      4x    10
//      8x    11
//
// resolution | bits [3:2] values
// ------------------------------
//    12 bits   00
//    14 bits   01
//    16 bits   10
//    18 bits   11
//
// conversion mode | bit [4] value
// -------------------------------
//        one-shot   0
//      continuous   1

// set rest of default config bits here
#define ADC_RDY   0x80  // 7th bit high initiates conversion in one-shot mode
#define ADC_CHANS 0x00  // single-channel ADC, so bits 5 and 6 are zero
#define ADC_MODE  0x00  // one-shot mode
#define ADC_PGA   0x00  // unity gain

// ADC configuration
static uint8_t adcConfig = ADC_RDY | ADC_CHANS | ADC_MODE | ADC_RES | ADC_PGA;

// buffer to hold bytes returned from the ADC
static uint8_t buff[4];

// private (local) functions
static void startConversion(void);
static int32_t getData(void);
static bool conversionBusy(void);
static double get_sensor_voltage(void);
static double get_battery(void);
static double sensor_max(double);

// returns 1 on success, 0 on failure
int adc_setup(void)
{
    // setup for master mode, pins 18/19, external pullups, 400kHz, 200ms default timeout
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
    Wire.setDefaultTimeout(200000); // 200ms

    delay(500);
    
    Wire.beginTransmission(ADC_ADDRESS);
    Wire.write(adcConfig);
    Wire.endTransmission(I2C_STOP);

    if ( Wire.getError() )
      return 0;

    return 1;
}


// voltage-to-angle conversion
// ---------------------------
// volts (x) to degrees (y) is a linear equation: y = mx + b
//
// with a 5V input, the angle sensor's nominal output voltage range is [0.2, 4.8],
// which is divided down before the ADC to [0.08, 1.92]. the min value corresponds to
// 0 degrees, and the max to 90 degrees. so, solving for the slope,
//
//    m = 90 / (1.92 - 0.08)
//
// and the intercept is given by
//
//    b = -0.08m
//
// therefore,
//
//    y = (90 / (1.92 - 0.08)) * x - (90 / (1.92 - 0.08) * 0.08
//      = (90 * x - 7.2) / (1.92 - 0.08)
//
// however, when the sensor input falls below 5V (which corresponds to a battery voltage
// of about 5.125V), the sensor's max output becomes a linear function of the battery
// voltage.  thus,
//
//    y = (90 * x - 7.2) / (vmax - 0.08)
//
// where vmax = 1.92 (for battery > 5.125), or vmax = 0.383*battery - 0.064.

void get_angle(void)
{
    double voltage; // angle sensor output voltage
    double vbat;    // battery voltage
    double vmax;    // sensor's maximum output

    vbat    = get_battery();
    vmax    = sensor_max( vbat );
    voltage = get_sensor_voltage();

    // set the global 'angle' var
    angle = (90.0L * voltage - 7.2L) / (vmax - 0.08L);
    angle += angle_offset;

    if ( angle < 0 ) angle = 0.0;
    
    return;
}

// the idea here is to give the user a way to zero the angle sensor for a more
// precise measurement.  while in the laser-aiming state, pressing the mode button
// calls this function, which takes an angle measurement and saves the offset for
// future calculations
void zero_angle(void)
{
    get_angle();
    angle_offset = 0.0 - angle;
}

// query the ADC for the angle sensor voltage
static double get_sensor_voltage(void)
{
    int32_t sum = 0;
    int32_t result;
    uint16_t count;

    for ( count = 0; count < WINDOW_SIZE; count++ )
    {
        startConversion();
        sum += getData();
    }
  
#if WINDOW_SIZE > 1
    // round the averaged result
    result = (sum + (WINDOW_SIZE / 2)) / WINDOW_SIZE;
#else
    result = sum;
#endif

    return ((double)result * LSB);
}

// getData() blocks while waiting for a conversion to finish, parses
// the data bytes (see datasheet for the gory details), and returns the
// resulting code
static int32_t getData(void)
{
    uint8_t  sign1; // holds 8 sign-extended bits
    uint16_t sign2; // holds 16 sign-extended bits
    int32_t  data;  // stores the conversion code
    
    // wait for conversion to be finished
    while ( conversionBusy() == true );

    // only 18-bit data needs special handling
    switch (RESOLUTION)
    {
      case 18:
        sign1 = buff[0] & 0x80 ? 0xFF : 0;
        data = (sign1 << 24) + (buff[0] << 16) + (buff[1] << 8) + buff[2];
        break;
    
      default:
        sign2 = buff[0] & 0x80 ? 0xFFFF : 0;
        data = (sign2 << 16) + (buff[0] << 8) + buff[1];
        break;
    }

    return data;
}

// conversionBusy() reads from the ADC and checks the /RDY flag
static bool conversionBusy(void)
{
    uint8_t i = 0;
    bool conversion_busy;

    Wire.requestFrom(ADC_ADDRESS, PACKET_SIZE);

    while( Wire.available() )
        buff[i++] = Wire.readByte();

    // the /RDY flag (7th bit in config) is low when conversion done
    conversion_busy = buff[PACKET_SIZE - 1] >> 7;

    return conversion_busy;
}

// a single-shot conversion is triggered by writing a 1 to the /RDY bit in the config
static void startConversion(void)
{
    Wire.beginTransmission(ADC_ADDRESS);
    Wire.write(adcConfig |= 128);
    Wire.endTransmission(I2C_STOP);

    if (Wire.getError())
        Serial.print("WRITE FAIL in startConversion()\n");
}

// the angle sensor's max voltage output scales with the battery voltage
// once the battery falls below 5.125V.  this function returns the battery
// voltage as calculated from the (internal) ADC count; we multiply by two
// because the actual voltage is halved by a resistor divider before the adc
// sees it:
//    voltage = count * LSB * 2
//            = count * (3.3/2^10) * 2
//            = count * 0.0064453125
static double get_battery(void)
{
    return ((double)analogRead(bat_pin) * 0.0064453125L);
}

// maximum sensor output is nominally 1.92V (after the voltage divider). however,
// once battery voltage falls below 5.125V, sensor max becomes a linear function
// of the battery voltage.  the function in the else clause is a linear interpolation
// derived from measurements.
static double sensor_max(double vbat)
{
    if ( vbat > 5.125 )
      return 1.92L;
    else
      return (0.383L * vbat - 0.064L);
}
