/* Small test program to check the GeoLoc CC embedded code meets the spec.
 * This is not a comprehensive test but a decent functional test.
 * This program also checks the examples in the documentation.
 * Date: 2024-12-02 - DrZWave@DrZWave.blog
 */

#include <stdio.h>
#include <stdlib.h>
#include "ZAF_types.h"
#include "CC_GeographicLoc.h"

//#define NO_DEBUGPRINT
//#define DPRINT(...) do {} while(0)

// This is an array of GPS data to be fed into the GeoLocCC code for testing purposes
unsigned char GPSData[] = {
"$GNGGA,155511.686,,,,,0,0,,,M,,M,,*5A\r\n Good CRC but no satellites - should be ignored$$$*** \x00" \
"Eiffel Tower: $GPGGA,220333.093,4851.542,N,00217.669,E,1,12,1.0,37.0,M,0.0,M,,*55\r\n" \
"Death Valley: $GPGGA,220333.093,3613.846,N,11647.047,W,1,12,1.0,-86.9,M,0.0,M,,*64\n" \
"Sydney Opera House: $GNGGA,221800.175,3351.398,S,15112.920,E,1,12,1.0,4.214,M,0.0,M,,*6F\n\n\x20\x00\x02\x03\x0a\xFF" \
"Christ Redeemer Statue: $GPGGA,221800.175,2257.114,S,04312.624,W,1,12,1.0,703.0333,M,0.0,M,,*5E   \r\r  " \
"McMurdo Station: $GPGGA,221800.175,7750.807777,S,16640.261234,E,1,12,1.0,118.111,M,0.0,M,,*78\r\n \x00" \
"Bad CRC        : $GPGGA,221800.175,7750.807777,S,16640.261234,E,1,12,1.0,118,M,0.0,M,,*66\r\n" \
"Bad CRC        : $GPGGA,221800.175,7750.807777,S,16640.261234,E,1,12,1.0,118,M,0.0,M,,*68\r\n" \
"Bad CRC        : $GPGGA,221800.175,7750.807777,S,16640.261234, E,1,12,1.0,118,M,0.0,M,,*67\r\n\x00" \
};


bool Check_not_legal_response_job(RECEIVE_OPTIONS_TYPE_EX *rxOpt) { // not testing multicast
    return(false);
}
uint32_t CORE_EnterAtomic(void) { // ignore
    return(0);
}
void CORE_ExitAtomic(uint32_t dummy) { // ignore
}

unsigned char * NMEA_sentence;
void NMEA_Init( uint8_t * ptr) {
    NMEA_sentence=ptr;
}

uint8_t buf[128]; // buffer for processing the NMEA sentence

bool checkOK( int32_t lat, int32_t lon, int32_t alt) { // compares the current coordinates with the hardcoded values passed in and returns false if it fails
    if ((GetLatitude() == lat) && (GetLongitude() == lon) && (GetAltitude() == alt)) return(true);
    printf("FAIL! Lat Expected %08x got %08x ", lat, GetLatitude());
    printf("Lon Expected %08x got %08x ", lon, GetLongitude());
    printf("Alt Expected %06x got %06x \r\n", alt, GetAltitude());
    return(false);
}

int main(void) {
    int index = 0;
    int index_last = 0;
    int TestNum = 1;
    printf("Testing GeoLocCC:\r\n");
    NMEA_Init(buf);
    while (index<sizeof(GPSData)) {
        while ((false==NMEA_build(GPSData[index++])) && (index<sizeof(GPSData))); // exits when we have a NMEA sentence in buf
        if (index<sizeof(GPSData)) {
            NMEA_parse(); // update the coordinates based on the data in the buffer
            printf("Test#%d Index=%d ", TestNum,index); 
            //printf(" %s ", &GPSData[index_last]);
            printf("Lat=%08x %f ",GetLatitude(), ((float)GetLatitude())/(1<<23));
            printf("Lon=%08x %f ",GetLongitude(), ((float)GetLongitude())/(1<<23));
            printf("Alt=%06x %f \r\n",GetAltitude(), ((float)GetAltitude())/100);
            switch (TestNum++) {
                case 1: if (!checkOK(0x7FFFFFFF, 0x7FFFFFFF, 0xFF800000)) exit(1); break;
                case 2: if (!checkOK(0x186df4cd, 0x0125b1a1, 0xe7a)) exit(1); break; // these were computed manually
                case 3: if (!checkOK(0x121d89c3, 0xc59ba211, 0xffffde07)) exit(1); break;
                case 4: if (!checkOK(0xef1259d7, 0x4b9b900a, 0x01a5)) exit(1); break;
                case 5: if (!checkOK(0xf4862825, 0xea65119d, 0x01129f)) exit(1); break;
                case 6: if (!checkOK(0xd9139c2e, 0x5355e400, 0x002e23)) exit(1); break;
                case 7: if (!checkOK(0x7FFFFFFF, 0x7FFFFFFF, 0xFF800000)) exit(1); 
                            if (0x10!=(0xf0 & GetStatus())) {
                                printf("checksum failed but Qual=%x, expected 0x1\r\n",GetStatus()>>4);
                                exit(1);
                            }
                        break;
                case 8: if (!checkOK(0x7FFFFFFF, 0x7FFFFFFF, 0xFF800000)) exit(1); 
                            if (0x10!=(0xf0 & GetStatus())) {
                                printf("checksum failed but Qual=%x, expected 0x1\r\n",GetStatus()>>4);
                                exit(1);
                            }
                        break;
                case 9: if (!checkOK(0x7FFFFFFF, 0x7FFFFFFF, 0xFF800000)) exit(1); 
                            if (0x10!=(0xf0 & GetStatus())) {
                                printf("checksum failed but Qual=%x, expected 0x1\r\n",GetStatus()>>4);
                                exit(1);
                            }
                        break;
                default: printf("not coded yet\r\n"); break;
            }
            index_last=index+1;
        }
    }
    printf("Tests PASS\r\n");
    exit(0);
}
