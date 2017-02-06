/*
 * Longitude MCP3421 ADC driver
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
// the angle sensor output range is [0.2, 4.8], and the voltage divider before
// the ADC represents a gain of 0.4.  so the ADC input range is [0.08, 1.92].
double get_angle(void)
{
    int32_t result;
    int32_t sum;
    uint16_t count;
    double voltage;
    double theta;
    double m = (double)90 / (1.92 - 0.08); // angle to volts slope
    double b = -m * 0.08; // linear offset (because 0 degrees == 0.08 V)

    sum = 0;
  
    for ( count = 0; count < WINDOW_SIZE; count++ )
    {
        startConversion();
        sum += getData();
    }
  
    // handle rounding
    if ( WINDOW_SIZE > 1 )
    {
        result = (sum + (WINDOW_SIZE / 2)) / WINDOW_SIZE;
    }
    else
    {
        result = sum;
    }
  
    voltage = (double)result * LSB;
    theta = m * voltage + b;

    return theta;
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

    if(Wire.getError())
        Serial.print("WRITE FAIL in startConversion()\n");
}
