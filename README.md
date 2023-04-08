# UiTM Smart Environmental Monitoring, Surveillance and Analytics (Public, Environmental and Occupational Health)
UiTM EMSA Technology

This is the official step for my class:

# Chapter 1
Step for Arduino IDE Setup:
1) Navigate arduino website: https://arduino.cc/software
2) Download the software according to your Operating System (OS)
3) Install the Arduino Software including the related USB Driver
4) Copy the following link: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
5) Go to File > Preferences > URL/ Board Manager, paste the above link followed by ","
6) Click OK
7) Then go to Tool Menu > Board > Board Managers
8) In the board manager, type: ESP32
9) Click install on the ESP32 by Espressif
10) Done Installation

# Chapter 2
Installation of Libraries
*There are two ways to install the libraries, first using the Arduino Library Manager, second using the folder/ zip file.

## Method 1 (Install from Library Manager)
1) Go to Sketch Menu > Include Library > Manage Libraries
2) Install the library as you need. 

## Method 2 (Install from zip file)
1) Navigate the source
2) Download the Zip file
3) Extract the Zip file
4) Rename the folder, remove the "-master" extension if any
5) Copy the unzip folder into Arduino library folder
6) Paste the folder into Arduino > libraries

## Method 2.1 (Install from the Github/ Repo)
1) Navigate to the link
2) Download
3) Extract
4) Rename the folder, remove the "-master" name/ extension (if any)
5) Copy the unzip and rename folder
6) Paste into the Arduino > libraries folder

## List of Libraries:
1) rtclib
2) rtc by makuna
3) MQunifiedsensor
4) Adafruit_GFX
5) Adafruit_SSD1306
6) Adafruit_PM25AQI
7) ThingSpeak
8) WiFiManager
9) SD card if not present link: https://github.com/espressif/arduino-esp32

# Chapter 3
1) Identify the urban environmental health analytics issues to be explore
2) Perform secondary data analysis (screening)
3) Priotitize the location of interest and start prototyping
4) Refer following connection [<img src="https://github.com/ismailsakdo/uitm_emsa/blob/main/Connection_bb.png" width="500px">]
5) Test each of the sensor using one module at one time
6) Follow Chapter 4

# Chapter 4
## Datalogger
The datalogger is define as the sistem/ operation that enable the user to record the data according to the time and date for particular sensor/ sensing process. Its enable the investigator to understand the trend associated with the date and time perspective. The main aim of the datalogger was to simulate the forecasting of the pollutants that exist with the ambient environment.

## Step of building datalogger
