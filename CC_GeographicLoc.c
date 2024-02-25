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
#include "UART_DRZ.h" // GPS receiver is on EUSART1

// Comment this out if NOT connected to a GPS receiver and only stores the location via SET.
#define GPS_ENABLED

#ifdef GPS_ENABLED
 #define GEO_READ_ONLY 1
#else
 #define GEO_READ_ONLY 0
#endif

// uncomment to get some printouts
#define DEBUGPRINT
#include "DebugPrint.h"

// GPS variables
static int32_t longitude=0;
static int32_t latitude=0;
static int32_t altitude=0;
static uint8_t alt_valid=0;
static uint8_t long_valid=0;
static uint8_t lat_valid=0;
static uint8_t gps_quality=0;

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
            output->frame->ZW_GeographicLocationReportV2Frame.altitude  = altitude;
            output->frame->ZW_GeographicLocationReportV2Frame.status    = ((0<<7)|(gps_quality<<5)|(GEO_READ_ONLY<<3)|(alt_valid<<2)|(lat_valid<<1)|(long_valid));
            output->length = sizeof(ZW_GEOGRAPHIC_LOCATION_REPORT_V2_FRAME); // triggers the send
            break;
        case GEOGRAPHIC_LOCATION_SET_V2:
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
}

static void reset(void)
{
  // called when a factory reset is performed - reset the NVM?
}

REGISTER_CC_V5(COMMAND_CLASS_GEOGRAPHIC_LOCATION, GEOGRAPHIC_LOCATION_VERSION_V2, CC_GeographicLoc_handler, NULL, NULL, lifeline_reporting, 0, init, reset);
// No BASIC CC mapping so those are NULL. zero is reserved field
