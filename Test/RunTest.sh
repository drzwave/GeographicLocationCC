# Shell script for testing the Geographic Location Command Class code
gcc GeoLocCC_Test.c ../CC_GeographicLoc.c -o geotest -g -DNO_DEBUGPRINT -I ./ -I../ -I/mnt/c/Users/eric/SimplicityStudio/SDKs/SDK202462/protocol/z-wave/ZAF/ApplicationUtilities -I/mnt/c/Users/eric/SimplicityStudio/SDKs/SDK202462/protocol/z-wave/dist/include/zwave/ -I/mnt/c/Users/eric/SimplicityStudio/SDKs/SDK202462/protocol/z-wave/dist/include/zpal/ -I/mnt/c/Users/eric/SimplicityStudio/SDKs/SDK202462/util/third_party/freertos/kernel/include/ -I/mnt/c/Users/eric/SimplicityStudio/SDKs/SDK202462/util/third_party/freertos/kernel/portable/GCC/ARM_CM33_NTZ/non_secure -I/mnt/c/Users/eric/SimplicityStudio/SDKs/SDK202462/protocol/z-wave/Components/QueueNotifying/ -I/mnt/c/Users/eric/SimplicityStudio/SDKs/SDK202462/protocol/z-wave/Components/NodeMask/ -I/mnt/c/Users/eric/SimplicityStudio/SDKs/SDK202462/platform/emlib/inc/ -I/mnt/c/Users/eric/SimplicityStudio/SDKs/SDK202462/platform/common/inc/ -I/mnt/c/Users/eric/SimplicityStudio/SDKs/SDK202462/protocol/z-wave/Components/DebugPrint/
if [ 0 -eq $? ]
then
	./geotest
fi
#gcc GeoLocCC_Test.c -o geotest -B /mnt/c/Users/eric/SimplicityStudio/SDKs/SDK202462/protocol/z-wave/ZAF/ApplicationUtilities 
