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


# Journal 
This section is temporary and will be deleted once its working. Just a place for me to keep some notes.


- 2024-03-25 - Project compiles - installed into SwOnOff\_ZGM230\_441\_GeoCC project for initial debug
- Purchased new GPS modules as they are now have QWIIC connectors so they can simply plug directly into a TBZ or ZReach
- But in the short term I need to use the UART based one I have now
- I have a WSTK with the header for the GPS module but the project gets completely stuck initializing the BURTC because there is no LFXO on the board I am using.
- Thus, I need to recreate the project again using this WSTK and a RB4250 which is what I have available. Originally I had used a TBZ (2603) which has a 32khz crystal. But now I need to plug in the GPS into a WSTK I already soldered the header into. This will be easier when I get the QWIIC GPS receivers.
- So once again starting from scratch in SSv5 because too much is done automagically based on the devkit and trying to fix the project just breaks it more so it's easier to start over.
- I switched back to my github Workspace where I have a lot more other projects and where the GeoCC repo is located.
- 
-
