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
#define GPS_ENABLED

#ifdef GPS_ENABLED
 #define GEO_READ_ONLY 1
#else
 #define GEO_READ_ONLY 0
#endif

bool NMEA_build(char c); // add a character to the NEMA Sentence buffer, return TRUE if complete sentence is in buffer
bool NMEA_checksum(void) ; // TRUE if the Sentence Checksum is good
void NMEA_parse(void);
int NMEA_getLongitude(void);
int NMEA_getLatitude(void);
int NMEA_getaltitude(void);

void NMEA_Init(uint8_t * ptr); // Initialize the pointer to the NMEA buffer in the specific hardware interface

#endif
