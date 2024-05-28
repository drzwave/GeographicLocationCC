/**
 * @file CC_GeographicLoc.c 
 * @brief Z-Wave Geographic Location V2 Command Class implementation for End Devices
 * See the ReadMe.md for more details.
 * @author Eric Ryherd drzwave@silabs.com
 * @date 2024/2/23
 *
 * Software Architecture:
 * There are two implementations in this code which are chosen based on the GPS_ENABLED #define in the CC_GeographicLoc.h file:
 *  1) GPS_ENABLED defined
 *      - A GPS receiver is attached via UART or I2C and provides live GPS data.
 *      - SET commands are ignored and coordinates are read-only as they come from the GPS receiver.
 *  2) GPS_ENABLED not defined
 *      - GPS coordinates are from a SET command which is typically from a mobile phone or other GPS device.
 *      - The GPS coordinates are stored in NVM.
 * 
 * The code in this file handles the GeoLocCC Set/Get/Report frames in the CC_GeographicLoc_handler.
 * The hardware interface to GPS receviers are in other files in this repo but utilize functions in this file.
 * The NMEA "sentence" from the GPS receiver is buffered and parsed in this file and converted into Long/Lat/Alt values each time the GPS sends data.
 * 
 */

// ISSUES:
// how to update ZW_classcmd.h?
// how to update the XML?
// add code for storing in NVM

#include "CC_GeographicLoc.h"
#ifdef GEOLOCCC_INTERFACE_UART
#include "UART_DRZ.h" // GPS receiver is on EUSART1
#endif
#include <ZW_TransportEndpoint.h>
#include <ZW_application_transport_interface.h>
#include <em_core_generic.h>    // CORE_ATOMIC

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
static uint8_t gps_quality=0;    // TODO - need to implement this!
//static uint8_t gps_time[]="HH:MM:SS";
//static uint8_t gps_satelites=0;

// Max length is 255
#define SENTENCE_BUF_LENGTH 100
static uint8_t SentenceBufRaw[SENTENCE_BUF_LENGTH]; // holds the GPS sentence for later parsing
static uint8_t NMEA_index; // index pointer into the buffer

int32_t NMEA_getAltitude(void);

/*
 * @brief handler for Geographic Location Command class when a command is received via Z-Wave
 */
static received_frame_status_t CC_GeographicLoc_handler(
    cc_handler_input_t * input,
    cc_handler_output_t * output)
{
    CORE_DECLARE_IRQ_STATE; // save irqstate when in CORE_ATOMIC

    switch (input->frame->ZW_Common.cmd)
    {
        case GEOGRAPHIC_LOCATION_GET_V2:
            if (true == Check_not_legal_response_job(input->rx_options)) {   // check for multicast etc.
                return RECEIVED_FRAME_STATUS_FAIL;
            }
            // send the report
            output->frame->ZW_GeographicLocationReportV2Frame.cmdClass = COMMAND_CLASS_GEOGRAPHIC_LOCATION_V2;
            output->frame->ZW_GeographicLocationReportV2Frame.cmd      = GEOGRAPHIC_LOCATION_REPORT_V2;
            CORE_ENTER_ATOMIC(); /* prevent values from changing while assembling the frame to avoid corruption */
            output->frame->ZW_GeographicLocationReportV2Frame.longitude1 = (uint8_t)((longitude>>23)&0xFF);
            output->frame->ZW_GeographicLocationReportV2Frame.longitude2 = (uint8_t)((longitude>>16)&0xFF);
            output->frame->ZW_GeographicLocationReportV2Frame.longitude3 = (uint8_t)((longitude>>8)&0xFF);
            output->frame->ZW_GeographicLocationReportV2Frame.longitude4 = (uint8_t)((longitude>>0)&0xFF);
            output->frame->ZW_GeographicLocationReportV2Frame.latitude1  = (uint8_t)((latitude>>23)&0xFF);
            output->frame->ZW_GeographicLocationReportV2Frame.latitude2  = (uint8_t)((latitude>>16)&0xFF);
            output->frame->ZW_GeographicLocationReportV2Frame.latitude3  = (uint8_t)((latitude>>8)&0xFF);
            output->frame->ZW_GeographicLocationReportV2Frame.latitude4  = (uint8_t)((latitude>>0)&0xFF);
            output->frame->ZW_GeographicLocationReportV2Frame.altitude1  = (uint8_t)((altitude>>16)&0xFF);
            output->frame->ZW_GeographicLocationReportV2Frame.altitude2  = (uint8_t)((altitude>>8)&0xFF);
            output->frame->ZW_GeographicLocationReportV2Frame.altitude3  = (uint8_t)((altitude>>0)&0xFF);
            output->frame->ZW_GeographicLocationReportV2Frame.status    = ((0<<7)|(gps_quality<<5)|(GEO_READ_ONLY<<3)|(alt_valid<<2)|(lat_valid<<1)|(long_valid));
            output->length = sizeof(ZW_GEOGRAPHIC_LOCATION_REPORT_V2_FRAME); /* triggers the send */
            CORE_EXIT_ATOMIC();
            break;
        case GEOGRAPHIC_LOCATION_SET_V2:
            // TODO more to come here - reject it if read only
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

// called by ZAF_Init() - Anything that needs initialization on reset - pick up values out of NVM or init the UART
static void init(void)
{
#ifdef GPS_ENABLED
  NMEA_Init(SentenceBufRaw); // initialize the pointer to the NMEA buffer which the GPS interface will fill in
#endif
}

// called when a factory reset is performed
static void reset(void)
{
    // TODO reset the values in the NVM?
}

typedef enum {  // NMEA state machine states
    NMEA_search,
    NMEA_GNGGA,
    NMEA_fetch,
    NMEA_checksum1,
    NMEA_checksum2,
    NMEA_error
} NMEA_state_e;

static uint8_t NMEAState = NMEA_search; // state machine that assembles the Sentance from a UART

/* @brief Add the character C to the "sentence" buffer as each byte arrives via a UART or I2C
 * returns true when a complete buffer has been filled otherwise false
 * Typical NMEA sentence: 
 * $GPGGA,121017.00,4310.24176,N,07052.27544,W,1,08,1.10,00048,M,-032,M,,*52
 *       ,TIME     , Lat        , Long        , ,  ,    , Alt   ,       ,Checksum
 */
bool NMEA_build(char c) {
    bool rtn = false;
    //DPRINTF("%c",c);
    switch(NMEAState) {
        case NMEA_search: // search for the $
            if ('$'==c) {
                NMEA_index=0;
                SentenceBufRaw[NMEA_index++]=c;
                NMEAState=NMEA_GNGGA;
            }
            break;
        case NMEA_GNGGA: // check for the desired sentence - GPGGA or GNGGA
            if ('G'==c || 'P'==c || 'N'==c || 'A'==c || ','==c) { 
                SentenceBufRaw[NMEA_index++]=c;
                if (','==c) {
                    NMEAState=NMEA_fetch;
                }
            } else { // ignore the other sentences
                NMEAState=NMEA_search;
            }
            break;
        case NMEA_fetch: // collect the sentence to the *
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
        DPRINT("!");
            rtn=true;
            break;
        default:
            NMEAState=NMEA_search;
            break;
    }
    return(rtn);
}

/* @brief convert hex string of up to 8 chars to an integer. Zero if non-hex chars are found.
 */
uint32_t hextoi(uint8_t * ptr) {
    uint32_t rtn = 0;
    for (uint8_t i=0; i<8; i++) {
        uint8_t c = ptr[i];
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

/* @brief return TRUE if the sentence checksum is OK
 */
bool NMEA_checksum(void) {
    uint32_t i;
    uint8_t sum=0;
    for (i=1; ((i<SENTENCE_BUF_LENGTH) && ('*'!=SentenceBufRaw[i])); i++) {
        sum ^= SentenceBufRaw[i];
    }
    SentenceBufRaw[i+3]='\0'; // NULL the end of the string
    int check = hextoi(&SentenceBufRaw[i+1]);
    //DPRINTF("\nSum=%x, Check=%x ",sum,check);
    if (check == sum) return(true);
    else return(false);
}

/* @brief parse the NMEA sentence and if checksum is OK, update the GPS coordinates 
 */
void NMEA_parse(void) {
    if (NMEA_checksum()) { // checksum is good so compute the coordinates
        latitude  = NMEA_getLatitude();
        if (0==latitude) lat_valid=0; 
        else lat_valid=1;
        longitude = NMEA_getLongitude();
        if (0==longitude) long_valid=0; 
        else long_valid=1;
        altitude  = NMEA_getAltitude();
        if (0==altitude) alt_valid=0; 
        else alt_valid=1;
    }
}

/* @brief search thru the NMEA sentence and return the 32 bit longitude a signed fixed point decimal degress with 23 bits of fraction
 * Returns 0 if errors
 */
int NMEA_getLongitude(void) {
    int i;
    int fieldNum = 0;
    int rtn = 0;
    int k;
    float t;
    char tmp[10];
    for (i=0;i<SENTENCE_BUF_LENGTH;i++) { // search for the longitude field which is the 2nd one
        if (','==SentenceBufRaw[i]) {
            fieldNum++;
            if (fieldNum==4) {
                i++;
                break;
            }
        }
    }
    if (SENTENCE_BUF_LENGTH<=i) return(0); // didn't find the field - return 0
    tmp[0]=SentenceBufRaw[i++];     // first 3 digits are degrees
    tmp[1]=SentenceBufRaw[i++]; 
    tmp[2]=SentenceBufRaw[i++]; 
    tmp[3]='\0';
    rtn = atoi(tmp)<<23; // decimal degrees
    DPRINTF("long D=%s ",tmp);
    for (k=0;((k+i)<SENTENCE_BUF_LENGTH) && (','!=SentenceBufRaw[k+i]);k++) {
        tmp[k]=SentenceBufRaw[k+i];     // copy to tmp buffer
    }
    tmp[k+1] = '\0';
    DPRINTF("M=%s",tmp);
    t = atof(tmp);
    rtn = rtn + (int32_t)(t*(float)(1<<23)/60); // convert from minutes to degrees and shift up 23 bits
    DPRINTF(" frac=%f hex=%X \r\n",t,rtn);
    if ('W'==SentenceBufRaw[i+1]) rtn=0-rtn; // West=negative
    if (0==rtn) rtn=1; // in the rare case when exactly 0, make it 1 as it is not an error
    return(rtn);
}

/* @brief search thru the NMEA sentence and return the 32 bit latitude a signed fixed point decimal degress with 23 bits of fraction
 * Returns 0 if errors
 */
int NMEA_getLatitude(void) {
    int i;
    int fieldNum = 0;
    int rtn = 0;
    int k;
    float t;
    char tmp[10];
    for (i=0;i<SENTENCE_BUF_LENGTH;i++) { // search for the latitude field which is the 2nd one
        if (','==SentenceBufRaw[i]) {
            fieldNum++;
            if (fieldNum==2) {
                i++;
                break;
            }
        }
    }
    if (SENTENCE_BUF_LENGTH<=i) return(0); // didn't find the field - return 0
    tmp[0]=SentenceBufRaw[i++];     // first 2 digits are degrees
    tmp[1]=SentenceBufRaw[i++]; 
    tmp[2]='\0';
    rtn = atoi(tmp)<<23; // decimal degrees
    DPRINTF("lat D=%s",tmp);
    for (k=0;((k+i)<SENTENCE_BUF_LENGTH) && (','!=SentenceBufRaw[k+i]);k++) {
        tmp[k]=SentenceBufRaw[k+i];     // copy to tmp buffer
    }
    tmp[k+1] = '\0';
    DPRINTF("M=%s",tmp);
    t = atof(tmp);
    rtn = rtn + (int32_t)(t*(float)(1<<23)/60); // convert from minutes to degrees and shift up 23 bits
    DPRINTF(" frac=%f hex=%X ",t,rtn);
    if ('S'==SentenceBufRaw[i+1]) rtn=0-rtn; // South=negative
    if (0==rtn) rtn=1; // in the rare case when exactly 0, make it 1 as it is not an error
    return(rtn);
}

/* @brief search thru the NMEA sentence and return the signed 32 bit altitude in cm
 * Returns 0 if errors
 */
int32_t NMEA_getAltitude(void) {
    int32_t i;
    int32_t fieldNum = 0;
    int32_t rtn = 0;
    int32_t k;
    float t;
    char tmp[10];
    for (i=0;i<SENTENCE_BUF_LENGTH;i++) { // search for the altitude field which is the 9th one
        if (','==SentenceBufRaw[i]) {
            fieldNum++;
            if (fieldNum==9) { // altitude is the 9th field
                i++;
                break;
            }
        }
    }
    if (SENTENCE_BUF_LENGTH<=i) return(0); // didn't find the Altitude - return 0
    for (k=0;((k+i)<SENTENCE_BUF_LENGTH) && (','!=SentenceBufRaw[k+i]);k++) {
        tmp[k]=SentenceBufRaw[k+i];     // copy the altitude to tmp buffer
    }
    tmp[k+1] = '\0';
    t = atof(tmp);
    //DPRINTF("alt=%s = %f \r\n",tmp,t);  // https://community.silabs.com/s/article/floating-point-print-with-gcc?language=en_US required to enable printing of floats (adds 5K of flash!)
    rtn=(int32_t)(t*100);   // convert to centimeters and return an integer
    if (0==rtn) rtn=1; // in the rare case when exactly at sealevel, return 1cm as it is not an error
    return(rtn);
} // NMEA_getAltitude


// This adds the command class to the NIF and links in the handler
REGISTER_CC_V5(COMMAND_CLASS_GEOGRAPHIC_LOCATION, GEOGRAPHIC_LOCATION_VERSION_V2, CC_GeographicLoc_handler, NULL, NULL, lifeline_reporting, 0, init, reset);
// No BASIC CC mapping so those are NULL. zero is reserved field
