/**
 * @file CC_GeographicLoc.h
 */

#ifndef CC_GEOGRAPHIC_LOCATION_H_
#define CC_GEOGRAPHIC_LOCATION_H_

#include <ZAF_types.h>

// Choose one of the 2 hardware interfaces below and uncomment ONE line
//#define GEOLOCCC_INTERFACE_I2C
//#define GEOLOCCC_INTERFACE_UART
// Choose the I2C interface by default as it is easier to connect to devkits
#if !defined GEOLOCCC_INTERFACE_I2C || !defined GEOLOCCC_INTERFACE_UART
#define GEOLOCCC_INTERFACE_I2C
#endif

// Comment this out if NOT connected to a GPS receiver and only stores the location via SET.
//#define GPS_ENABLED

#ifdef GPS_ENABLED
 #define GEO_READ_ONLY 1
#else
 #define GEO_READ_ONLY 0
#endif

// Default values when VALID=0 are invalid values
#define LAT_DEFAULT 0x7FFFFFFF
#define LON_DEFAULT 0x7FFFFFFF
#define ALT_DEFAULT 0xFF800000

#ifdef GPS_ENABLED
bool NMEA_build(char c); // add a character to the NEMA Sentence buffer, return TRUE if complete sentence is in buffer
bool NMEA_checksum(void) ; // TRUE if the Sentence Checksum is good
void NMEA_parse(void);
int32_t NMEA_getLongitude(void);
int32_t NMEA_getLatitude(void);
int32_t NMEA_getAltitude(void);
int32_t NMEA_getStatus(void);
int32_t GetLatitude(void);
int32_t GetLongitude(void);
int32_t GetAltitude(void);
int32_t GetStatus(void);

void NMEA_Init(uint8_t * ptr); // Initialize the pointer to the NMEA buffer in the specific hardware interface

#else // Non GPS enabled

typedef struct SgpsCoordinates  // file structure for storing the GPS coordinates from a SET command
{
    int32_t longitude;
    int32_t latitude;
    int32_t altitude; // note that altitude is actually only 24 bits but it's easier to define it as an int23
} SgpsCoordinates;

#endif

#endif
