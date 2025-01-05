# Z-Wave Geographic Location Command Class Version 2

Z-Wave Geographic Location CC V2 adds the ability to report or store the GPS coordinates of a device with centimeter resolution.
Sensor networks using Z-Wave Long Range (ZWLR) can be spread across an area of over 12 square miles.
A reliable method of locating sensors in this large area is provided using Geographic Location Command Class Version 2.

The accuracy of most low-cost GPS receivers is only a few meters. 
Expensive GPS receivers can achieve centimeter resolution but those would typically not be used in an Internet of Things (IoT) application.
There are two implementations of Geographic Location: 1) A device with its own GPS receiver; 2) a device which is assigned its GPS location at commissioning time.
GPS receivers are expensive both in cost and power consumption. Applications such as tracking the movement of glaciers, tracking of farm equipment, or any application where the node will move will include a GPS receiver.
Lower cost applications such as sensor networks may utilize a phone app during commissioning to assign the GPS location of the sensor. 
Moisture/temperature sensors in a vineyard or orchard would use this lower cost method where the location is fixed at commissioning time.

GeoLocCC can simplify RF Range testing as the exact GPS location of the device can be reported back to the controller as it is moved around an area.
Heat maps can be generated showing exactly where the DUT is on a map with additional data such as transmit power and RSSI.

## Z-Wave Geographic Location Command Class V1 is obsolete

Geographic Location Command Class V2 obsoletes Version 1 as no Z-Wave device was certified with Geographic Location V1.
Version 1 has only seven bits of resolution resulting in 1+ kilometer of resolution. 

## Geographic Location V2 Features:

  - Signed fixed point decimal longitude and latitude encoding with 23 bits of fraction yielding centimeter resolution
  - Altitude in centimeters
  - Status byte with validity bits for each value, read-only bit and GPS quality fields

# Getting Started

Clone this repository either directly into a project or into an adjacent folder and use softlinks for the specific files needed.
Then follow the step-by-step instructions below based on the GPS receiver interface being used: UART or I2C.

# GPS Receiver Hardware Interfaces

- GPS receivers typically use either a UART or I2C:
- UART:
    - UARTs do not have standardized connectors and require custom wiring
    - Using a WSTK ProKit with a Z-Wave radio board would require wiring to the header next to the slide switch 
        - WSTK Pin 1=VMCU=3.3V
        - WSTK Pin 3=GND
        - WSTK Pin 6=P1=ZG23 PC1=RX
        - WSTK Pin 8=P3=ZG23 PC2=TX
    - Using a DK2603 (Thunderboard) devkit would wire a similar set of pins to the holes along the edge of the board
- I2C:
    - The DK2603 DevKit and the Z-Wave Alliance Reference Application Device (ZRAD) both have QWIIC connectors on them
    - [Sparkfun](https://www.sparkfun.com/) sells several GPS receivers with QWIIC connectors
        - [SAM-M8Q](https://www.sparkfun.com/products/15210) - $43
        - [SAM-M10Q](https://www.sparkfun.com/products/21834) - $50
        - [NEO-M9N](https://www.sparkfun.com/products/15712) - $70 (requires external U.FL antenna)
        - [ZED-F9P](https://www.sparkfun.com/products/15136) - $275 claims to have 10mm accuracy, requires U.FL antenna
        - [XA1110](https://www.sparkfun.com/products/14414) - $35 - Seems to get lost when it loses lock and has significant errors when it relocks if the DUT is moving. Poor atlitude accuracy. Not recommended.

The source code provided in this repo has code specific to each interface.
The first thing to do is to choose which interface you will use and then uncomment one of the 2 defines in CC\_GeographicLoc.h to choose which interface will be used.

# Step-by-Step Installation into the SwitchOnOff sample project using SSDK 2024.6.2

1. Create the sample app, set the RF Region, enable debug printing
    - Check that it builds - ideally download and join a network and send On/Offs
    - Recommend also installing Z-Wave Debug Print and uncommenting #define DEBUGPRINT in app.c
2. Copy (or softlink) the .c and .h files of this repo into the project 
    - Do not copy the CC\_GeographicLoc1.h - the contents of this file must be pasted into ZW\_classcmd.h in the next step
    - Softlinks can be created using a bash shell started with administrator rights and then use the "ln -s <target>" command on windows
3. Since Geographic Location CC V2 has not been released yet, SDK ZW\_classcmd.h file must be customized to add support for GeoLocCCv2:
3. Edit the file simplicity\_sdk\_202x.x.x/protocol/z-wave/dist/include/zwave/ZW\_classcmd.h
    - SSv5 will ask if you want to make a copy - click on "Make a Copy"
        - For a more permanent solution, edit the version in the SDK which will allow you to make multiple projects from this SDK version
    - Go to line 691 and copy the contents of CC\_GeographicLoc1.h into the ZW\_cmdclass.h file
        - Must be copied in due to the backslashes - cannot #include
        - Just after Geographic Location V1 - search for "geograph" to find the correct line
    - Go to line 6411 and enter: #include "CC\_GeographicLoc2.h"
        - just after Geographic Location V1
    - Go to line 22732 and enter: #include "CC\_GeographicLoc3.h" 
        - just after Geographic Location V1 Set frame
    - optionally move the file to the top level with the other project files so it is easy to see what has changed
    - Once GeoLoc V2 is released into the SDK this step won't be required

- UART Interface:
    - MORE TO COME HERE - TBD - 
    - Edit events.h
        - add "EVENT_EUSART1_CHARACTER_RECEIVED," to the end of the enum EVENT_APP_SWITCH_ON_OFF
        - the project should build without errors at this point
    - Replace the app.c in the sample project with the one from the repo

- I2C Interface:
    - Click on the .slcp file - select the Software Components tab - enter I2CSPM into the seach bar
        - Click on Platform->Driver->I2C->I2CSPM and Install it
        - Name the component GPS
        - Click on Configure
        - Reference clock frequency=0 (default), Speed mode=Fast mode (400kbit/s), 
            - Selected Module=I2C0 (or I2C1), 
            - Select the IOs which for ZRAD are SCL=PB00, SDA=PB02
            - Select the IOs which for Thunderboard (BRD2603) are SCL=PB05, SDA=PB06
        - The project should build OK - the I2C peripheral will be automatically initialized
        - The I2CSPM_Transfer() function is then used to send/receive data over the I2C bus
    - Follow the instructions in GPS module file (SAM\_M8Q.c) to install the code into app.c 

# Adding Geographic Location CC V2 without hardware

- add instructions here...

# Technical Information

GPS data from common GPS receivers provides longitude, latitude and altitude data in the form of a "NMEA Sentence".
Data is typically transmitted over a UART at 4800 baud once per second. 
Often the data and formats can be configured.
Some GPS recivers use I2C for serial data transfer. The data is the same but the bus master must poll the GPS receiver to get the data.

# Geographic Location Report command

<figure class="wp-block-table"><table><tbody><tr><td class="has-text-align-center" data-align="center">7</td><td>6</td><td>5</td><td>4</td><td>3</td><td>2</td><td>1</td><td>0</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Command Class = COMMAND_CLASS_GEOGRAPHIC_LOCATION (0x8C)</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Command = GEOGRAPHIC_LOCATION_REPORT (0x03)</td></tr><tr><td class="has-text-align-center" data-align="center">Lo Sign</td><td colspan="7">Longitude Integer[8:1]</td></tr><tr><td class="has-text-align-center" data-align="center">Lo[0]</td><td colspan="7">Long Fraction[22:16]</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Longitude Fraction[15:8]</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Longitude Fraction[7:0]</td></tr><tr><td class="has-text-align-center" data-align="center">La Sign</td><td colspan="7">Latitude Integer[8:1]</td></tr><tr><td class="has-text-align-center" data-align="center">La[0]</td><td colspan="7">Lat Fraction[22:16]</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Latitude Fraction[15:8]</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Latitude Fraction[7:0]</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Altitude[23:16] MSB in cm</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Altitude[15:8]</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Altitude[7:0] LSB</td></tr><tr></td><td colspan="4">Qual</td><td>RO</td><td>Al Valid</td><td>La Valid</td><td>Lo Valid</td></tr></tbody></table><figcaption class="wp-element-caption">The SET command (0x01) is the same as REPORT without the STATUS byte. The GET command remains the same as V1.</figcaption></figure>

## Longitude

Longitude is a 32-bit fixed point signed decimial value from -180.0 to +180.0. The sign is the most significant bit. The next 8 bits are the integer portion. The remaining 23 bits are the fraction. Zero is at the prime meridian in Greenwich England. Positive values are east of the prime meridian, negative values are west. Longitude MUST be zero when the LoValid bit is zero. Longitude should not be exactly zero when LoValid bit is a one. In the rare case where the reading is exactly at the prime meridian, add 1 cm to make the value just slightly non-zero.

Longitude can be converted to a floating point value using Java with a command similar to: `const longitude= payload.readInt32BE(0) / (1 << 23);`. See the sample code for how to convert the NMEA Sentence into the binary format.

## Latitude

Latitude is a 32-bit fixed point signed decimial value from -90.0 to +90.0. The sign is the most significant bit. The next 8 bits are the integer portion. The remaining 23 bits are the fraction. Zero is at the equator with north being positive and south being negative. The rules for Longitude apply to Latitude and conversion is similar.

## Altitude

Altitude is a signed 24-bit integer value in _centimeters_ above mean sea level. A value of exactly zero MUST be returned if the AlValid bit is zero. If exactly at sea level, the value should be adjusted to 1 cm to avoid an exactly zero value when AlValid is 1. Note the altitude is less accurate in most GPS receivers than the longitude or latitude. Typical altitude accuracy is 10 meters or less in receivers with 1 meter longitude/latitude resolution. Conversion using Java is similar to: `const alt = payload.readIntBE(8, 3) / 100;` where the alt object is now a floating point value in meters. Note the binary value is 3 bytes long.

## Status Byte

The Status Byte has several fields which provide additional information on the other values. The STATUS byte is read-only and is NOT included in a SET command.

### LoValid

LoValid is a one when the Longitude value is valid. Zero when it is not. When RO is set to 1 and the GPS receiver is locked to sufficient satelites, LoValid is 1. When LoValid is 0, the Longitude value is NOT valid and must be ignored. When RO is zero, LoValid is zero after factory reset or exclusion. LoValid MUST be one after receiving a Geographic Location SET command with a valid non-zero value for Longitude. Setting Longitude to exactly zero clears LoValid to zero.

### LaValid

LaValid is identical to LoValid except it relates to Latitude instead of Longitude.

### RO - Read Only

RO is set to one when a GPS receiver is attached to the system indicating the Longitude, Latitude and Altitude values are Read-Only and cannot be SET. When RO=1, SET commands are ignored. RO is zero when the geographic location values are set from an external device (typically a phone application during commissioning).

### Qual

The Quality field MUST be zero when the RO bit is 0. In systems with a GPS receiver, the QUAL field is an indicator of the signal quality of the GPS signal. A value of 0 indicates no GPS signal and the coordinates SHOULD be ignored (the Valid bits should be zero). The QUAL field typically contains the number of satellites in use with the last reading. Four satellites are required for an accurate reading. If more than 15 satellites are in use, the QUAL field is set to 15. Recommendation is to use values 0-3 as error codes: 0=no GPS receiver communication indicating hardware failure, 1=NMEA checksum failure indicating communication errors (out of sync or buffer over/under runs), 2 and 3 are user defined. 

# Reference Documents

- [How To Implement a New Command Class](https://docs.silabs.com/z-wave/7.21.2/zwave-api/md-content-how-to-implement-a-new-command-class) - docs.silabs.com
    - Switch to the latest version by clicking on "Version History" or google it
- [NMEA GPS](https://www.gpsworld.com/what-exactly-is-gps-nmea-data/) Sentence definition

# Status

- Code the read/write version which stores the coords in NVM instead of read-only via a GPS receiver

