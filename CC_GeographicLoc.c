/**
 * @file GeographicLocCC.c 
 * @brief Z-Wave Geographic Location V2 Command Class implementation for End Devices
 * See the ReadMe.md for more details.
 * @author Eric Ryherd drzwave@silabs.com
 * @date 2024/2/23
 */

// ISSUES:
// how to update ZW_classcmd.h?
// how to update the XML?
// implement the GPS receiver code - ISR function and UART details
// add code for storing in NVM

#include "CC_GeographicLoc.h"
#ifdef GEOLOCCC_INTERFACE_UART
#include "UART_DRZ.h" // GPS receiver is on EUSART1
#endif
#include <ZW_TransportEndpoint.h>


// uncomment to enable debugging info
#define DEBUGPRINT
#include "DebugPrint.h"
#include <string.h>
#include <stdlib.h>

// GPS variables
static int32_t longitude=0;
static int32_t latitude=0;
static int32_t altitude=0;
static uint8_t alt_valid=0;
static uint8_t long_valid=0;
static uint8_t lat_valid=0;
static uint8_t gps_quality=0;
//static uint8_t gps_time[]="HH:MM:SS";
//static uint8_t gps_satelites=0;

// Max length is 255, 80 bytes is more than enough
#define SENTENCE_BUF_LENGTH 80
static char SentenceBufRaw[SENTENCE_BUF_LENGTH]; // holds the GPS sentence for later parsing
//static uint8_t NMEA_index; // index pointer into the buffer
/*
 * @brief handler for Geographic Location Command class when a command is received via Z-Wave
 */
static received_frame_status_t CC_GeographicLoc_handler(
    cc_handler_input_t * input,
    cc_handler_output_t * output)
{
    switch (input->frame->ZW_Common.cmd)
    {
        case GEOGRAPHIC_LOCATION_GET_V2:
            if (true == Check_not_legal_response_job(input->rx_options)) {
                return RECEIVED_FRAME_STATUS_FAIL;
            }
            // send the report
            output->frame->ZW_GeographicLocationReportV2Frame.cmdClass = COMMAND_CLASS_GEOGRAPHIC_LOCATION_V2;
            output->frame->ZW_GeographicLocationReportV2Frame.cmd      = GEOGRAPHIC_LOCATION_REPORT_V2;
            output->frame->ZW_GeographicLocationReportV2Frame.longitude = longitude;
            output->frame->ZW_GeographicLocationReportV2Frame.latitude  = latitude;
            output->frame->ZW_GeographicLocationReportV2Frame.altitude  = altitude; // TODO altitude is a 24 bit value but this is 32
            output->frame->ZW_GeographicLocationReportV2Frame.status    = ((0<<7)|(gps_quality<<5)|(GEO_READ_ONLY<<3)|(alt_valid<<2)|(lat_valid<<1)|(long_valid));
            output->length = sizeof(ZW_GEOGRAPHIC_LOCATION_REPORT_V2_FRAME); // triggers the send
            break;
        case GEOGRAPHIC_LOCATION_SET_V2:
            // more to come here - reject it if read only
            break;
        default:
            return RECEIVED_FRAME_STATUS_NO_SUPPORT;
            break;
    }
  return RECEIVED_FRAME_STATUS_SUCCESS;
}

#ifdef GPS_ENABLED
// GPS conversion to GeoLocCC
#else
// Store the GPS values set in NVM
#endif

static uint8_t lifeline_reporting(ccc_pair_t * p_ccc_pair)
{
  p_ccc_pair->cmdClass = COMMAND_CLASS_GEOGRAPHIC_LOCATION;
  p_ccc_pair->cmd      = GEOGRAPHIC_LOCATION_REPORT_V2;
  return 1;
}

static void init(void)
{
  // called by ZAF_Init() - Anything that needs initialization on reset - pick up values out of NVM or init the UART
  NMEA_Init(SentenceBufRaw); // initialize the pointer to the NMEA buffer which the GPS interface will fill in
}

static void reset(void)
{
  // called when a factory reset is performed - reset the NVM?
}

#ifdef GEOLOCCC_INTERFACE_UART
typedef enum {  // NMEA state machine states
    NMEA_search,
    NMEA_fetch,
    NMEA_checksum1,
    NMEA_checksum2,
    NMEA_error
} NMEA_state_e;

static uint8_t NMEAState = NMEA_search; // state machine that assembles the Sentance from a UART

/* @brief Add the character C to the "sentence" buffer as each byte arrives via a UART
 * returns true when a complete buffer has been filled otherwise false
 * Typical NMEA sentence: 
 * $GPGGA,121017.00,4310.24176,N,07052.27544,W,1,08,1.10,00048,M,-032,M,,*52
 *        TIME     , Lat        , Long        , ,  ,    , Alt   ,       ,Checksum
 */
bool NMEA_build(char c) {
    bool rtn = false;
    switch(NMEAState) {
        case NMEA_search: // search for the $
            if ('$'==c) {
                NMEA_index=0;
                SentenceBufRaw[NMEA_index++]=c;
                NMEAState=NMEA_fetch;
            }
            break;
        case NMEA_fetch: // collect the sentence from the $ to the *
            if (NMEA_index>=SENTENCE_BUF_LENGTH-3) { // don't overrun the buffer
                NMEAState=NMEA_search;
            } else if ('*'==c) { // start of checksum
                NMEAState=NMEA_checksum1;
            } 
            SentenceBufRaw[NMEA_index++]=c;
            break;
        case NMEA_checksum1: // capture checksum 1st digit
            NMEAState=NMEA_checksum2;
            SentenceBufRaw[NMEA_index++]=c;
            break;
        case NMEA_checksum2: // capture checksum 2nd digit
            NMEAState=NMEA_search;
            SentenceBufRaw[NMEA_index++]=c;
            rtn=true;
            break;
        default:
            NMEAState=NMEA_search;
            break;
    }
    return(rtn);
}
#endif

/* @brief convert hex string of up to 8 chars to an integer. Zero if non-hex chars are found.
 */
uint32_t hextoi(char * ptr) {
    uint32_t rtn = 0;
    for (uint8_t i=0; i<8; i++) {
        char c = ptr[i];
        if ('\0' == c) break; // end of string = done
        if (c >= '0' && c <='9') c = c - '0';
        else if (c >= 'A' && c <= 'F') c = c - 'A' +10;
        else if (c >= 'a' && c <= 'f') c = c - 'a' +10;
        else { // non hex digit - return 0
            c=0;
            rtn=0;
            break;
        }
        rtn = (rtn<<4) | (c &0x0f); // shift in the nibble
    }
    return(rtn);
}

/* @brief return TRUE if the sentence starts with $GPGGA or $GNGGA
 */
bool NMEA_GPGGA(void) { 
    if ((0==strncmp("$GPGGA",&SentenceBufRaw[0],6U)) ||
        (0==strncmp("$GNGGA",&SentenceBufRaw[0],6U))
       ) return(true);
    else return(false);
}

/* @brief return TRUE if the sentence checksum is OK
 */
bool NMEA_checksum(void) {
    uint32_t i;
    uint8_t sum=0;
    for (i=1; ((i<SENTENCE_BUF_LENGTH) && ('*'!=SentenceBufRaw[i])); i++) {
        sum ^= SentenceBufRaw[i];
    }
    SentenceBufRaw[i+3]='\0'; // NULL the end of the string
    DPRINTF("\ni=%d, buf[i]=%s ",i,&SentenceBufRaw[i]);
    int check = hextoi(&SentenceBufRaw[i+1]);
    DPRINTF("\nSum=%x, Check=%x ",sum,check);
    if (check==sum) DPRINT("GOOD!\n");
    else DPRINT(" BAD!!!\n");
    if (check == sum) return(true);
    else return(false);
}

/* @brief search thru the NMEA sentence and return the 32 bit latitude a signed fixed point decimal degress with 23 bits of fraction
 * Returns 0 if errors
 */
int NMEA_getLatitude(void) {
    int i;
    int fieldNum = 0;
    int rtn = 0;
    int k,m,n;
    char tmp[10];
    for (i=0;i<SENTENCE_BUF_LENGTH;i++) { // search for the latitude field which is the 2nd one
        if (','==SentenceBufRaw[i]) {
            fieldNum++;
            if (fieldNum==2) {
                i++;
                break;
            }
            if (SENTENCE_BUF_LENGTH==i) return(0);
            tmp[0]=SentenceBufRaw[i++]; 
            tmp[1]=SentenceBufRaw[i++]; 
            tmp[2]='\0';
            rtn = atoi(tmp)<<23; // decimal degrees
            tmp[0]=SentenceBufRaw[i++]; 
            tmp[1]=SentenceBufRaw[i++]; 
            k = atoi(tmp)<<23; // decimal minutes
            if ('.'!=SentenceBufRaw[i++]) return(0);
            for (m=i; ((m<SENTENCE_BUF_LENGTH) && (','!=SentenceBufRaw[i+m]));m++) {
               tmp[m] = SentenceBufRaw[i+m]; 
            }
            tmp[m]='\0';
            n = atoi(tmp)*(10*m);
        }
    }
}

REGISTER_CC_V5(COMMAND_CLASS_GEOGRAPHIC_LOCATION, GEOGRAPHIC_LOCATION_VERSION_V2, CC_GeographicLoc_handler, NULL, NULL, lifeline_reporting, 0, init, reset);
// No BASIC CC mapping so those are NULL. zero is reserved field
