# Z-Wave Geographic Location Command Class Version 2

Z-Wave Geographic Location CC V2 adds the ability to report or store the
GPS coordinates of a device to within centimeter resolution.
Typically the accuracy of most GPS receivers is at best a few meters, the centimeter resolution is more than enough.

# Getting Started

- How to use this Repo???
- How do you install this into a project?
    - Just copy the files into the project directory and they are in?

# Setup

- Adding a GPS receiver to a Silabs devkit is realtively easy using a standard QWIIC connector
- Most GPS receivers use a simple UART to send data once/second to an MCU
- But the UARTs are not standardized and thus require custom wiring making them less reliable
- [Sparkfun](https://www.sparkfun.com/) sells several GPS receivers with QWIIC connectors
    - [XA1110](https://www.sparkfun.com/products/14414) - GPS Breakout
- If using the UART interface:
    - WSTK Pin 1=VMCU=3.3V
    - WSTK Pin 3=GND
    - WSTK Pin 6=P1=ZG23 PC1=RX
    - WSTK Pin 8=P3=ZG23 PC2=TX

# Step-by-Step Installation into the SwitchOnOff sample project using GSDK 4.4.1

1. Create the sample app, set the RF Region, enable debug printing
    - Check that it builds - ideally download and join a network and send On/Offs
2. Copy (or softlink) the .c and .h files of this repo into the project 
    - Do not copy the CC\_GeographicLoc1.h - the contents of this file must be pasted into ZW\_classcmd.h in the next step
    - Softlinks can be created using a bash shell started with administrator rights and then use the "ln -s <target>" command
3. Edit the file: gecko\_sdk\_4.4.1/protocol/z-wave/PAL/inc/ZW\_classcmd.h
    - SSv5 will ask if you want to make a copy - click on "Make a Copy"
    - Go to line 691 and copy the contents of CC\_GeographicLoc1.h into the ZW\_cmdclass.h file
        - Must be copied in due to the backslashes - cannot #include
        - Just after Geographic Location V1
    - Go to line 6408 and enter: #include "CC\_GeographicLoc2.h"
        - just after Geographic Location V1
    - Go to line 22729 and enter: #include "CC\_GeographicLoc3.h" 
        - just after Geographic Location V1 Set frame
    - optionally move the file to the top level with the other project files so it is easy to see what has changed
    - Once GeoLoc V2 is released into the SDK this step won't be required
4. Edit events.h
    - add "EVENT_EUSART1_CHARACTER_RECEIVED," to the end of the enum EVENT_APP_SWITCH_ON_OFF
    - the project should build without errors at this point
5. Replace the app.c in the sample project with the one from the repo


# Technical Information

GPS data from common GPS receivers provides longitude and latitude data in the form of a "NMEA Sentence".
Data is typically transmitted over a UART at 4800 baud once per second. 
Often the data and formats can be configured.
Some GPS recivers use I2C for serial data transfer. The data is the same but the bus master must poll the GPS receiver to get the data.

<figure class="wp-block-table"><table><tbody><tr><td class="has-text-align-center" data-align="center">7</td><td>6</td><td>5</td><td>4</td><td>3</td><td>2</td><td>1</td><td>0</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Command Class = COMMAND_CLASS_GEOGRAPHIC_LOCATION (0x8C)</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Command = GEOGRAPHIC_LOCATION_REPORT (0x03)</td></tr><tr><td class="has-text-align-center" data-align="center">Lo Sign</td><td colspan="7">Longitude Integer[8:1]</td></tr><tr><td class="has-text-align-center" data-align="center">Lo[0]</td><td colspan="7">Long Fraction[22:16]</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Longitude Fraction[15:8]</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Longitude Fraction[7:0]</td></tr><tr><td class="has-text-align-center" data-align="center">La Sign</td><td colspan="7">Latitude Integer[8:1]</td></tr><tr><td class="has-text-align-center" data-align="center">La[0]</td><td colspan="7">Lat Fraction[22:16]</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Latitude Fraction[15:8]</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Latitude Fraction[7:0]</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Altitude[23:16] MSB in cm</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Altitude[15:8]</td></tr><tr><td class="has-text-align-center" data-align="center" colspan="8">Altitude[7:0] LSB</td></tr><tr><td class="has-text-align-center" data-align="center">RSV</td><td colspan="3">Qual</td><td>RO</td><td>Al Valid</td><td>La Valid</td><td>Lo Valid</td></tr></tbody></table><figcaption class="wp-element-caption">REPORT is the same as SET with the additional STATUS byte</figcaption></figure>

# Reference Documents

- [HowTo Implement a New Z-Wave Command Class](https://docs.silabs.com/z-wave/7.21.1/zwave-api/md-content-how-to-implement-a-new-command-class) - docs.silabs.com
- [NMEA GPS](https://www.gpsworld.com/what-exactly-is-gps-nmea-data/) Sentence definition
- 
- 

# Status

- Install GeoLoc CC into a SwOnOff project
- Connect a GPS receiver to a TBZ or DevKit
- Get data out of the GPS
- Convert data to GeoLocCC format
- Code Python script to talk to a serialAPI to get GeoCC every 30s and place it in a .csv file which can then be plotted on a map
- Get I2C GPS receivers to work
- Code the store format

# Journal 

This section is temporary and will be deleted once its working. Just a place for me to keep some notes.


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
