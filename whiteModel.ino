#include <Wire.h>
#include "SparkFun_ENS160.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//option display
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     // Reset pin # (or -1 if sharing reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

SparkFun_ENS160 myENS;
int ensStatus;

//add for datalogger function
#include <SPI.h>
#include <SD.h>
#include "RTClib.h"
#include "FS.h"
#include "SPI.h"

//Sensor dust
#include <SoftwareSerial.h>
#include "Adafruit_PM25AQI.h"

//Assignment for variable
const int chipSelect = 5;
File sd_file;
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
SoftwareSerial pmSerial(3, 2);  // change to RXD
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

//Variable
int pm03; // 0.3um/100mL
int pm05; // 0.5um/100mL
int pm25; // 2.5um/100mL
int pm10; // 10um/100mL

//additional
int pm50; // pm5.0
int pm01; // pm1.0

//Thingspeak library and WiFiManager
#include <WiFi.h>
#include <ThingSpeak.h>
#define SECRET_SSID "USERNAME"    // replace MySSID with your WiFi network name
#define SECRET_PASS "PASSWORD"  // replace MyPassword with your WiFi password
#define SECRET_CH_ID IDTHINGSPEAK                    // replace 0000000 with your channel number
#define SECRET_WRITE_APIKEY "APIWRITEKEy"  // replace XYZ with your channel write API Key

//Thingspeak + Internet
char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

//Variable Internet
String myStatus = "";

//variable datalogger
String dataMessage;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  if ( !myENS.begin() )
  {
    Serial.println("Did not begin.");
    while (1);
  }
  if ( myENS.setOperatingMode(SFE_ENS160_RESET) )
    Serial.println("Ready.");

  delay(100);
  myENS.setOperatingMode(SFE_ENS160_STANDARD);
  ensStatus = myENS.getFlags();
  Serial.print("Gas Sensor Status Flag: ");
  Serial.println(ensStatus);

  //Hidupkan sensor PMS
  pmSerial.begin(9600);
  if (!aqi.begin_UART(&pmSerial)) {  // connect to the sensor over software serial
    Serial.println("Could not find PM 2.5 sensor!");
    while (1) delay(10);
  }
  Serial.println("PM25 found!");

  // Initialize ThingSpeak
  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);

  //SD Card dihidupkan
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  initSDCard();

  //Hidupkan RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }


  // Create a file on the SD card and write the data labels
  File file = SD.open("/data.txt");
  if (!file) {
    Serial.println("File doesn't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/data.txt", "Date & Time, PM0.3, PM2.5, PM10, Other Gases \r\n");
  } else {
    Serial.println("File already exists");
  }
  file.close();

  //Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  display.display();
  delay(2);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0, 5);
  display.print("  AQ Meter ");
  display.display();
  delay(3000);
}

void loop()
{
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }
  
  
  myENS.checkDataStatus();
  Serial.print("Air Quality Index (1-5) : ");
  Serial.println(myENS.getAQI());

  Serial.print("Total Volatile Organic Compounds: ");
  Serial.print(myENS.getTVOC());
  Serial.println("ppb");

  Serial.print("CO2 concentration: ");
  Serial.print(myENS.getECO2());
  Serial.println("ppm");

  //Dust Sensor
  PM25_AQI_Data data;
  if (!aqi.read(&data)) {
    Serial.println("Could not read from AQI");
    delay(500);  // try again in a bit!
    return;
  }

  //Read Date, Time and PMS
  DateTime now = rtc.now();
  pm03 = data.particles_03um;
  pm05 = data.particles_05um;
  pm25 = data.particles_25um;
  pm10 = data.particles_100um;

  //additional
  pm50 = data.particles_50um;
  pm01 = data.particles_10um;

  //Read Date, Time and PMS
  Serial.print("Date:  ");
  Serial.print(now.day() + String("-") + now.month() + String("-") + now.year());
  Serial.print(", Time:  ");
  Serial.print(now.hour() + String(":") + now.minute() + String(":") + now.second());
  Serial.print(",CO2 (PPM):      ");
  Serial.print(myENS.getECO2());
  Serial.print(" TVOC: ");
  Serial.print(myENS.getTVOC());
  Serial.print(" Today is:       ");
  Serial.println(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.println("--------------------------------------------------------");

  //Concatenate all info separated by commas
  dataMessage = String(now.day()) + String("/") + String(now.month()) + String("/") + String(now.year()) + String("||") +
                String(now.hour()) + String(":") + String(now.minute()) + String(":") + String(now.second()) + String("||") +
                String(";") + myENS.getAQI() + String(";") + myENS.getTVOC() + String(";") + myENS.getECO2() +
                String(";") + pm03 + String(";") + pm01 + String(";") + pm05 + String(";") + pm50 + String(";") + pm25 + String(";") + pm10 + "\r\n";
  Serial.print("Saving data: ");
  Serial.println(dataMessage);

  //Append the data to file
  appendFile(SD, "/data.txt", dataMessage.c_str());


  //Thingspeak
  ThingSpeak.setField(1, myENS.getAQI());
  ThingSpeak.setField(2, myENS.getTVOC());
  ThingSpeak.setField(3, myENS.getECO2());
  ThingSpeak.setField(4, pm03);
  ThingSpeak.setField(5, pm01);
  ThingSpeak.setField(6, pm05);
  ThingSpeak.setField(7, pm25);
  ThingSpeak.setField(8, pm10);

  // set the status
  ThingSpeak.setStatus(myStatus);

  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    Serial.println("Channel update successful.");
  } else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

  //display OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(75, 0);
  display.println(now.second(), DEC);

  display.setTextSize(1);
  display.setCursor(25, 0);
  display.println(":");

  display.setTextSize(1);
  display.setCursor(65, 0);
  display.println(":");

  display.setTextSize(1);
  display.setCursor(40, 0);
  display.println(now.minute(), DEC);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(now.hour(), DEC);

  display.setTextSize(1);
  display.setCursor(0, 10);
  display.println(now.day(), DEC);

  display.setTextSize(1);
  display.setCursor(25, 10);
  display.println("-");

  display.setTextSize(1);
  display.setCursor(40, 10);
  display.println(now.month(), DEC);

  display.setTextSize(1);
  display.setCursor(55, 10);
  display.println("-");

  display.setTextSize(1);
  display.setCursor(70, 10);
  display.println(now.year(), DEC);

  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("CO2:");

  display.setTextSize(1);
  display.setCursor(30, 20);
  display.print(myENS.getECO2());

  display.setTextSize(1);
  display.setCursor(60, 20);
  display.print("TVOC:");

  display.setTextSize(1);
  display.setCursor(90, 20);
  display.print(myENS.getTVOC());  

  display.setTextSize(1);
  display.setCursor(0, 30);
  display.print("PM0.3:");

  display.setTextSize(1);
  display.setCursor(40, 30);
  display.print(pm03);
  
  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print("PM25:");

  display.setTextSize(1);
  display.setCursor(40, 40);
  display.print(pm25);

  display.setTextSize(1);
  display.setCursor(0, 50);
  display.print("PM10:");

  display.setTextSize(1);
  display.setCursor(40, 50);
  display.print(pm10);
  
  display.display();
  delay(15000);
}


//SD Card Setting


// Write to the SD card
void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

// Initialize SD card
void initSDCard() {
  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
}


// Append data to the SD card
void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
    //display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(100, 40);
    display.print("save");
    display.display();
    ;
  } else {
    Serial.println("Append failed");
    //display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(100, 40);
    display.print("NOT save");
    display.display();
  }
  file.close();
}
