#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MQUnifiedsensor.h>
#include <Adafruit_BMP280.h>

//add for datalogger function
#include <SPI.h>
#include <SD.h>
#include "RTClib.h"
#include "FS.h"
#include "SPI.h"

//Sensor dust
#include <SoftwareSerial.h>
#include "Adafruit_PM25AQI.h"

//Thingspeak library and WiFiManager
#include <WiFi.h>
#include <WiFiManager.h>
#include <ThingSpeak.h>
#define SECRET_CH_ID 00000                    // replace 0000000 with your channel number
#define SECRET_WRITE_APIKEY "API_WRITE_KEY"  // replace XYZ with your channel write API Key

//Thingspeak + Internet
unsigned long myChannelNumber = SECRET_CH_ID;
const char *myWriteAPIKey = SECRET_WRITE_APIKEY;
const char *server = "api.thingspeak.com";
WiFiClient client;

//Sensor IR
int detectPin = 4;
bool detect = 0;

//Assignment for variable
Adafruit_BMP280 bmp; // I2C
const int chipSelect = 5;
File sd_file;
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
SoftwareSerial pmSerial(3, 2);  // change to RXD
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

//option display
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     // Reset pin # (or -1 if sharing reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//intialize
#define Board ("Arduino NANO")
#define Pin135 (32)  //Analog input 2 of your arduino

//setup of sensor
#define RatioMQ135CleanAir (3.6)  //RS / R0 = 10 ppm
#define ADC_Bit_Resolution (12)  // 10 bit ADC
#define Voltage_Resolution (5)   // Volt resolution to calc the voltage
#define Type ("ESP32")           //Board used

//Initialize MQ
MQUnifiedsensor MQ135(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin135, Type);

//Variable
float co2;
float temp;
float pressure;
int pm03;
int pm05;
int pm25;
int pm10;

//variable datalogger
String dataMessage;

//Variable Internet
String myStatus = "";

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(115200);
  Wire.begin();

  //Hidupkan sensor IR
  pinMode(detectPin, INPUT);

  //Hidupkan sensor PMS
  pmSerial.begin(9600);
  if (!aqi.begin_UART(&pmSerial)) {  // connect to the sensor over software serial
    Serial.println("Could not find PM 2.5 sensor!");
    while (1) delay(10);
  }
  Serial.println("PM25 found!");

  // Initialize WiFiManager
  WiFiManager wifiManager;
  //wifiManager.resetSettings();
  wifiManager.autoConnect("Wataverse Technology", "password");

  // Initialize ThingSpeak
  ThingSpeak.begin(client);

  //Hidupkan sensor BMP280
  while ( !Serial ) delay(100);   // wait for native usb
  Serial.println(F("BMP280 test"));
  unsigned status;
  status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
  status = bmp.begin(0x76);
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                     "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(), 16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  //Hidupkan Sensor MQ
  MQ135.init();
  MQ135.setRegressionMethod(1);  //_PPM =  a*ratio^b
  MQ135.setR0(9.03);

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


void loop() {
  //IR
  detect = digitalRead(detectPin);
  
  //Dust Sensor
  PM25_AQI_Data data;
  if (!aqi.read(&data)) {
    Serial.println("Could not read from AQI");
    delay(500);  // try again in a bit!
    return;
  }

  //temp & pressure
  temp = bmp.readTemperature();  // get temp from SHT
  pressure = bmp.readPressure(); // get temp from SHT

  //MQ Sensor
  MQ135.update();
  MQ135.setA(110.47);
  MQ135.setB(-2.862);  //CO2
  float co2 = MQ135.readSensor();

  //Read Date, Time and PMS
  DateTime now = rtc.now();
  pm03 = data.particles_03um;
  pm05 = data.particles_05um;
  pm25 = data.particles_25um;
  pm10 = data.particles_100um;

  //Read Date, Time and PMS 
  Serial.print("Date:  ");
  Serial.print(now.day() + String("-") + now.month() + String("-") + now.year());
  Serial.print(", Time:  ");
  Serial.print(now.hour() + String(":") + now.minute() + String(":") + now.second());
  Serial.print(",CO2 (PPM):      ");
  Serial.print(co2);
  Serial.print(" Motion: ");
  Serial.print(detect);
  Serial.print(" Today is:       ");
  Serial.println(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.println("--------------------------------------------------------");

  //Concatenate all info separated by commas
  dataMessage = String(now.day()) + String("/") + String(now.month()) + String("/") + String(now.year()) + String("||") +
                String(now.hour()) + String(":") + String(now.minute()) + String(":") + String(now.second()) + String("||") +
                String(";") + temp + String(";") + pressure + String(";") + co2 + 
                String(";") + pm03 + String(";") + pm05 + String(";") + pm25 + String(";") + pm10 + 
                String(";") + detect + "\r\n";
  Serial.print("Saving data: ");
  Serial.println(dataMessage);

  //Append the data to file
  appendFile(SD, "/data.txt", dataMessage.c_str());

  //Thingspeak
  ThingSpeak.setField(1, temp);
  ThingSpeak.setField(2, pressure);
  ThingSpeak.setField(3, co2);
  ThingSpeak.setField(4, pm03);
  ThingSpeak.setField(5, pm05);
  ThingSpeak.setField(6, pm25);
  ThingSpeak.setField(7, pm10);
  ThingSpeak.setField(8, detect);

  // set the status
  ThingSpeak.setStatus(myStatus);

  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    Serial.println("Channel update successful.");
  } else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

  //IR sensor + Display Status
  if (detect != 1) {

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
  display.print("No Movement ");

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

  } else {
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
  display.print("Detected!!! ");

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

  }

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
