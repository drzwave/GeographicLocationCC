/** 
 * @file xa1110.c
 * @brief Support for the SparkFun XA1110 GPS QWIIC receiver in support of Geographic Command Class
 * 
 * This code will extract the GPS NMEA Sentence from a SparkFun XA1110 GPS receiver board.
 * The XA1110 is easy to use since it has a QWIIC connector so it can be easily added to any Silabs Devkit with a QWIIC connector (DK2603)
 * The tricky part is that the XA1110 has to be polled every 900ms to pull the GPS data via I2C.
 * The data comes across in pieces which must then be reassembled into a single string.
 * Then the Geographic Location Command Class code can then extract the coordinates from the NMEA string and report them via Z-Wave.
 *
 * Setup to add support to the Z-Wave SwitchOnOff sample app GSDK 4.4.1
 * #include the xa1110.h file into app.c
 * Add the following to the beginning of ApplicationTask after app_hw_init().
m_AppTaskHandle = xTaskGetCurrentTaskHandle();
AppTimerSetReceiverTask(m_AppTaskHandle);
AppTimerRegister(&I2CTimer, false, ZCB_I2CTimerCallBack);
TimerStart( &I2CTimer, XA1110_POLLING_INTERVAL);

 * add the following lines near the top of app.c 
#include "xa1110.h"
static SSwTimer I2CTimer;
static TaskHandle_t m_AppTaskHandle;

Add the following lines to ApplicationInit just before the user task creation
  AppTimerInit(EAPPLICATIONEVENT_TIMER,NULL); // XA1110 support

add the following line to the EVENT_APP_SWITCH_ON_OFF enum
  EVENT_APP_I2CTIMER_TIMEOUT,

 */


typedef enum {
  I2C_IDLE,
  I2C_GPGGA,
  I2C_DATA,
  I2C_CHECKSUM,
  I2C_ERROR
} i2c_state_t;

#include <sl_i2cspm.h> // I2C stuff
#include <sl_i2cspm_QWIIC_config.h>
#include <AppTimer.h>
#include "xa1110.h"
#include "events.h"
#define DEBUGPRINT
#ifdef DEBUGPRINT
#include <DebugPrint.h>
#endif

static uint8_t NMEA_sentence[NMEA_BUF_SIZE]; // Build the GPS NMEA sentence with the string needed - "$G.NSS..."

void Fetch_GPS(void) { // fetch the GPS NMEA sentence from the XA1110 GPS module over I2C and store it in the NMEA_sentence buffer
    static I2C_TransferSeq_TypeDef i2c_dat;
    static uint8_t i2c_txBuf[I2C_BUF_SIZE];
    static uint8_t i2c_rxBuf[I2C_BUF_SIZE];
    I2C_TransferReturn_TypeDef i2c_rtn;
    int NMEA_index=0;
    int i2c_read=0; // index into the Rx buffer
    bool i2c_done;
    bool i2c_fetch;
    int blankcount=0;

    i2c_state_t i2c_state = I2C_IDLE;

    i2c_dat.addr = 0x10<<1; // XA1110 I2C address=0x01 (7-bit)
    i2c_dat.flags = I2C_FLAG_READ;
    i2c_dat.buf[0].data= &i2c_rxBuf[0];
    i2c_dat.buf[0].len= sizeof(i2c_rxBuf);
    i2c_txBuf[0]=0;
    i2c_dat.buf[1].data= &i2c_txBuf[0];
    i2c_dat.buf[1].len= 0;
    //    i2c_rtn=I2CSPM_Transfer(SL_I2CSPM_QWIIC_PERIPHERAL, &i2c_dat);
    //    DPRINTF("\ni2c_rtn=%d - %02X - %s\n",i2c_rtn, i2c_rxBuf[0], i2c_rxBuf);
    i2c_done=false;
    i2c_fetch=true;
    while (!i2c_done) {
        if (i2c_fetch) {  // fetch another buffer of data
            i2c_rtn=I2CSPM_Transfer(SL_I2CSPM_QWIIC_PERIPHERAL, &i2c_dat);
            i2c_read=0;
            i2c_fetch=false;
            DPRINT(".");
        }
        switch (i2c_state) {
            case I2C_IDLE:
                NMEA_index=0;
                while ((i2c_read<I2C_BUF_SIZE) && (I2C_IDLE==i2c_state) && !i2c_done) {
                    if ('$'==i2c_rxBuf[i2c_read]) {
                        i2c_state=I2C_GPGGA;
                        NMEA_sentence[NMEA_index++] = i2c_rxBuf[i2c_read];
                    } else if (0x0a==i2c_rxBuf[i2c_read]) {
                        blankcount++;
                        if (blankcount>=I2C_BUF_SIZE) {
                            i2c_done=true; // full 32 bytes of 0x0a indicates there is no valid data - wait for the next sample
                            DPRINT("-");
                        }
                    } else {
                        blankcount=0;
                    }
                    i2c_read++;
                }
                if (i2c_read>=I2C_BUF_SIZE) {
                    i2c_fetch=true;
                }
                break;
            case I2C_GPGGA: // search for the $GPGGA (or $GNGGA) sentence
                while (('G'==i2c_rxBuf[i2c_read] || 'P'==i2c_rxBuf[i2c_read] || 'N'==i2c_rxBuf[i2c_read] || 'A'==i2c_rxBuf[i2c_read]) && !i2c_fetch) {
                    NMEA_sentence[NMEA_index++] = i2c_rxBuf[i2c_read++];
                    if (i2c_read>=I2C_BUF_SIZE) {
                        i2c_fetch=true;
                    }
                }
                if (NMEA_index>=6){ // got the $GPGGA - get the data
                    i2c_state=I2C_DATA;
                } else if (!i2c_fetch) { // not the right sentence - go back to searching for $
                    i2c_state=I2C_IDLE;
                }
                break;
            case I2C_DATA:
                while (('*'!=i2c_rxBuf[i2c_read]) && (i2c_read<I2C_BUF_SIZE)) {
                    if (0==i2c_rxBuf[i2c_read]) {   // TODO - checking for error cases - should not happen
                        DPRINT("Z");
                    }
                    if (' '==i2c_rxBuf[i2c_read]) { // TODO - also should not happen
                        DPRINT("K");
                    }
                    if (0x0A==i2c_rxBuf[i2c_read]) { // data is not available yet
                        blankcount++;
                        if (blankcount>=I2C_BUF_SIZE) i2c_done=true;
                        i2c_read++;
                    } else {
                        NMEA_sentence[NMEA_index++] = i2c_rxBuf[i2c_read++]; // copy the char
                    }
                } 
                if ('*'==i2c_rxBuf[i2c_read]) { // next 2 chars are the checksum
                    NMEA_sentence[NMEA_index++] = i2c_rxBuf[i2c_read++]; // copy the char
                    i2c_state=I2C_CHECKSUM;
                }
                if (i2c_read>=I2C_BUF_SIZE) {
                    i2c_fetch=true;
                }
                break;
            case I2C_CHECKSUM:
                if ((i2c_read+2)<I2C_BUF_SIZE) { // have all the data in this buffer
                    NMEA_sentence[NMEA_index++] = i2c_rxBuf[i2c_read++];
                    NMEA_sentence[NMEA_index++] = i2c_rxBuf[i2c_read++];
                    NMEA_sentence[NMEA_index]=0; // terminate the string
                    i2c_done=true; // all done
                    DPRINTF("NMEA=%s",NMEA_sentence);
                } // TODO - handle the case where the 2 checksum bytes are in the next buffer
                i2c_state=I2C_IDLE;
                break;
            default: // error - return to idle
                DPRINTF("i2c_state=%d i2c_read=%d\n",i2c_state,i2c_read);
                NMEA_sentence[NMEA_index]=0; // just to terminate the string
                DPRINTF("ERROR NMEA=%s\n",NMEA_sentence);
                i2c_state=I2C_IDLE;
                i2c_done = true; // exit to avoid watchdog
                break;
        }
    }
}

// This callback fetches the GPS coordinates every XA1110_POLLING_INTERVAL milliseconds
void ZCB_I2CTimerCallBack(SSwTimer *pTimer) {
  zaf_event_distributor_enqueue_app_event(EVENT_APP_I2CTIMER_TIMEOUT);
  DPRINT("\n+");
  Fetch_GPS();          // fetch the GPS NMEA Sentence from the XA1110
    // sometimes it fails to fetch the sentence in which case we just wait for the next interval
  TimerRestart(pTimer);
}
