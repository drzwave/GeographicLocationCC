/**
 * This is a temporary file that is #included at line 22729 in ZW_classcmd.h
 * once the CC V2 is approved it will be built-in
 * this file has the CC structures
 */

/************************************************************/
/* Geographic Location Get command class structs */
/************************************************************/
typedef struct _ZW_GEOGRAPHIC_LOCATION_GET_V2_FRAME_
{
    uint8_t   cmdClass;                     /* The command class */
    uint8_t   cmd;                          /* The command */
} ZW_GEOGRAPHIC_LOCATION_GET_V2_FRAME;

/************************************************************/
/* Geographic Location Report command class structs */
/************************************************************/
typedef struct _ZW_GEOGRAPHIC_LOCATION_REPORT_V2_FRAME_
{
    uint8_t   cmdClass;                     /* The command class */
    uint8_t   cmd;                          /* The command */
    int32_t   longitude;
    int32_t   latitude;
    int32_t   altitude;
    uint8_t   status;                       /* masked byte */
} ZW_GEOGRAPHIC_LOCATION_REPORT_V2_FRAME;

/************************************************************/
/* Geographic Location Set command class structs */
/************************************************************/
typedef struct _ZW_GEOGRAPHIC_LOCATION_SET_V2_FRAME_
{
    uint8_t   cmdClass;                     /* The command class */
    uint8_t   cmd;                          /* The command */
    int32_t   longitude;
    int32_t   latitude;
    int32_t   altitude;
} ZW_GEOGRAPHIC_LOCATION_SET_V2_FRAME;
