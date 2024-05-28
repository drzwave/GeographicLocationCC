# Z-Wave Geographic Location Command Class Version 2

Z-Wave Geographic Location CC V2 adds the ability to report or store the
GPS coordinates of a device to within centimeter resolution.
Common use-cases for Geographic Location CC are sensor networks using Z-Wave Long Range (ZWLR). 
ZWLR has over 2 miles of RF range which can result in an area of 16 square miles. Thus, a reliable method of locating sensors in this large area is needed for a variety of applications. 

Typically the accuracy of most GPS receivers is at best a few meters. 
Expensive GPS receivers can achieve centimeter resolution but those would typically not be used for IoT.
GeoLocCC is handy when performing RF Range testing as the actual location of the device can be reported if a GPS recevier is connected.
Heat maps can be generated showing exactly where the DUT is on a map and the signal quality.
Typical applications use a phone app to store the GPS coordinates to save on the cost of the GPS receiver connected to an IoT device.
The GPS coordinates can be recorded when the device is included into the network or at some point during the commissioning process once the final location has been established.

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
        - [XA1110](https://www.sparkfun.com/products/14414) - GPS Breakout

The source code provided in this repo has code specific to each interface.
The first thing to do is to choose which interface you will use and then uncomment one of the 2 defines in CC\_GeographicLoc.h to choose which interface will be used.

# Step-by-Step Installation into the SwitchOnOff sample project using GSDK 4.4.2

1. Create the sample app, set the RF Region, enable debug printing
    - Check that it builds - ideally download and join a network and send On/Offs
    - Recommend also installing Z-Wave Debug Print and uncommenting #define DEBUGPRINT in app.c
2. Copy (or softlink) the .c and .h files of this repo into the project 
    - Do not copy the CC\_GeographicLoc1.h - the contents of this file must be pasted into ZW\_classcmd.h in the next step
    - Softlinks can be created using a bash shell started with administrator rights and then use the "ln -s <target>" command
3. Since Geographic Location CC V2 has not been released yet, SDK ZW\_classcmd.h file must be customized to add support for GeoLocCCv2:
3. Edit the file: gecko\_sdk\_4.4.x/protocol/z-wave/PAL/inc/ZW\_classcmd.h
    - SSv5 will ask if you want to make a copy - click on "Make a Copy"
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
    - MORE TO COME HERE - TBD - this section needs to be rewritten...
    - Edit events.h
        - add "EVENT_EUSART1_CHARACTER_RECEIVED," to the end of the enum EVENT_APP_SWITCH_ON_OFF
        - the project should build without errors at this point
    - Replace the app.c in the sample project with the one from the repo

- I2C Interface:
    - Follow the instructions in the ZRAD repo to install I2CSPM
    - Follow the instructions in XA1110.c to install the code into app.c 

# Adding Geographic Location CC V2 without hardware

- add instructions here...

# Technical Information

GPS data from common GPS receivers provides longitude and latitude data in the form of a "NMEA Sentence".
Data is typically transmitted over a UART at 4800 baud once per second. 
Often the data and formats can be configured.
Some GPS recivers use I2C for serial data transfer. The data is the same but the bus master must poll the GPS receiver to get the data.

<figure class="wp-block-table"><table><tbody><tr><td class="has-text-align-center" data-align="center">7</td><td>6</td><td>5</td><td>4</td><td>3</td><td>2</td><td>1</td><td>0</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Command Class = COMMAND_CLASS_GEOGRAPHIC_LOCATION (0x8C)</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Command = GEOGRAPHIC_LOCATION_REPORT (0x03)</td></tr><tr><td class="has-text-align-center" data-align="center">Lo Sign</td><td colspan="7">Longitude Integer[8:1]</td></tr><tr><td class="has-text-align-center" data-align="center">Lo[0]</td><td colspan="7">Long Fraction[22:16]</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Longitude Fraction[15:8]</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Longitude Fraction[7:0]</td></tr><tr><td class="has-text-align-center" data-align="center">La Sign</td><td colspan="7">Latitude Integer[8:1]</td></tr><tr><td class="has-text-align-center" data-align="center">La[0]</td><td colspan="7">Lat Fraction[22:16]</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Latitude Fraction[15:8]</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Latitude Fraction[7:0]</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Altitude[23:16] MSB in cm - or make this 32 bits to match?</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Altitude[15:8]</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Altitude[7:0] LSB</td></tr><tr><td class="has-text-align-center" data-align="center">RSV</td><td colspan="3">Qual</td><td>RO</td><td>Al Valid</td><td>La Valid</td><td>Lo Valid</td></tr></tbody></table><figcaption class="wp-element-caption">REPORT is the same as SET with the additional STATUS byte</figcaption></figure>

# Reference Documents

- [How To Implement a New Command Class](https://docs.silabs.com/z-wave/7.21.2/zwave-api/md-content-how-to-implement-a-new-command-class) - docs.silabs.com
    - Switch to the latest version by clicking on "Version History" or google it
- [NMEA GPS](https://www.gpsworld.com/what-exactly-is-gps-nmea-data/) Sentence definition

# Status

- Currently getting bad data from the SDK - is the stack overflowing? RxOpts has 299 in it which is invalid!

- Install GeoLoc CC into a SwOnOff project
- Connect a GPS receiver to a TBZ or DevKit
- Get data out of the GPS
- Convert data to GeoLocCC format
- Code Python script to talk to a serialAPI to get GeoCC every 30s and place it in a .csv file which can then be plotted on a map
- Get I2C GPS receivers to work
- Code the read/write version which stores the coords in NVM instead of read-only via a GPS receiver

# Journal 

This section is temporary and will be deleted once its working. Just a place for me to keep some notes.


<b>2024-05-10</b> - Trace debugging bad data from the SDK

See the ZRAD tech documentation for details.


<b>2024-04-15</b> - I2C debug

The challenge is how and when to compute the Lon/Lat from the I2C bus data.
The XA1110 code has completed a sentence. It could generate an event with a pointer to the data and then use a 2nd buffer to start filling that one in.
The event is then handled later by app.c and converted into Lon/Lat/Alt values when requested or do it all the time?
Another option is to just wait until a GET comes in and then fetch it? That would probably take too long. Plus might have to wait for the GPS to send the next frame which might be 1-2 seconds later.
That's definitely too long. So have to keep at least getting the sentence every second.
Might as well also compute the Lon/Lat values. That should take very little time compared to the I2C bus.
I can add a Mutex so who owns the buffer is kept to one or the other.

<b>2024-03-29</b> - Computing the GeoLoc values from the GPS data

- converting a Latitude to GeoLocCC:
- 4851.54168,N = 48.859028 in decimal degrees = 409859233 in 8.23 fixed point =  0x18 6D F4 A1
- 4851.54168,N = decimal integer is 48. Just shift that up 23 bits. 0x1800\_0000
- 4851.54168,N = converting minutes into a decimal fraction is just divide by 60: 51.54168/60 = 0.859028
- 4851.54168,N = Shift integer up by 23 bits 51<<23 = 427819008 (0x19800000).
- 4851.54168,N = Add zeros to the fraction 7 digits then add that to the integer minutes 5416800 (0x52a760) + integer = 433235808 (0x19D2A760)
- 4851.54168,N = Divide by 60 = 7229596 (0x6e2d74)
- 4851.54168,N = 
- 4859.99999,N = 48.999999 in decimal degrees
- 4800.00000,N = 48.000000 in decimal degrees
- To convert the hex back into a floating point value in python:
- 0x184c4b3f = divide by 2\*\*23?
- 


<b>2024-04-11</b> - support the XA1110 module via QWIIC

- Added the files for the SparkFun XA1110 GPS module which is connected to ZRAD

<b>2024-03-28</b> - Calculating the CC values from the GPS data

- Capturing the GPGGA sentence and calculating the checksum correctly. Next up is to convert the values.

<b>2024-03-26</b> - GPS Data via UART

- Modified app.c to get the data. That will have to be part of the repo.
- GPS Sentence is: ```$GPGGA,121017.00,4310.24176,N,07052.27544,W,1,08,1.10,00048,M,-032,M,,*52```
- Time = UTC time HHMMSS.SS
- Latitude = ```DDMM.MMMMM,N``` DD are degrees, MM.MMM are minutes and a fraction, N is North or S for South which would be negative
- Longitude = ```DDDMM.MMMMM,W``` DDD are degrees, MM.MMM are minutes and a fraction, W is West which is negative or E for east
- Note that the number of digits in the fraction can vary
- Altitude = ```00048, M``` altitude in meters
- Checksum is the XOR of all characters after the $ and before \*

<b>2024-03-25</b> - Project compiles - installed into SwOnOff\_ZGM230\_441\_GeoCC project for initial debug

- Purchased new GPS modules as they are now have QWIIC connectors so they can simply plug directly into a TBZ or ZReach
- But in the short term I need to use the UART based one I have now
- I have a WSTK with the header for the GPS module but the project gets completely stuck initializing the BURTC because there is no LFXO on the board I am using.
- Thus, I need to recreate the project again using this WSTK and a RB4250 which is what I have available. Originally I had used a TBZ (2603) which has a 32khz crystal. But now I need to plug in the GPS into a WSTK I already soldered the header into. This will be easier when I get the QWIIC GPS receivers.
- So once again starting from scratch in SSv5 because too much is done automagically based on the devkit and trying to fix the project just breaks it more so it's easier to start over.
- I switched back to my github Workspace where I have a lot more other projects and where the GeoCC repo is located.
