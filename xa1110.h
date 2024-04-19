#ifndef XA1110_H_
#define XA1110_H_

#include "zaf_event_distributor_soc.h"

#define I2C_BUF_SIZE 32
#define NMEA_BUF_SIZE 80
// The polling interval should typically be under 1s to avoid overruns which would require even more error handling
#define XA1110_POLLING_INTERVAL 933

void Fetch_GPS(void);
void ZCB_I2CTimerCallBack(SSwTimer *pTimer);

#endif