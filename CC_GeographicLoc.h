/**
 * @file CC_GeographicLoc.h
 */

#ifndef CC_GEOGRAPHIC_LOCATION_H_
#define CC_GEOGRAPHIC_LOCATION_H_

#include <ZAF_types.h>

bool NMEA_build(char c); // add C to the NEMA Sentence buffer, return TRUE if complete sentence is in buffer
bool NMEA_GPGGA(void) ; // TRUE if Sentence starts with $GPGGA
bool NMEA_checksum(void) ; // TRUE if the Sentence Checksum is good

#endif
