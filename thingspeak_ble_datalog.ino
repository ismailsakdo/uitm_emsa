//ID-XXXXXXX
//THINGSPEAK-API

//setup the RTC using RTC by makuna
//Reference - https://github.com/ismailsakdo/EHRU_EMSA/blob/main/GROUP%20EHRU%20SKETCH22/nana_ccs811_mq.ino
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "RTClib.h"
#include "FS.h"
#include "SPI.h"
#include <MQUnifiedsensor.h>
#include <Adafruit_AHTX0.h>
//#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

//Option OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Thingspeak library and WiFiManager
#include <WiFiManager.h>
#include <ThingSpeak.h>
#define SECRET_CH_ID XXXXX                    // replace 0000000 with your channel number
#define SECRET_WRITE_APIKEY "APIWRITE"  // replace XYZ with your channel write API Key

//Sensor dust
#include <SoftwareSerial.h>
#include "Adafruit_PM25AQI.h"

//Sensor IR
int detectPin = 13;
bool detect = 0;

//Assignment
const int chipSelect = 5;
File sd_file;
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
SoftwareSerial pmSerial(3, 2);  // change to RXD
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
Adafruit_AHTX0 aht;
//BluetoothSerial SerialBT;

//option display
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     // Reset pin # (or -1 if sharing reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//intialize
#define Board ("Arduino NANO")
#define Pin135 (32)  //Analog input 2 of your arduino
//#define         Pin7                     (35)  //Analog input 3 of your arduino

//setup of sensor
#define RatioMQ135CleanAir (3.6)  //RS / R0 = 10 ppm
//#define         RatioMQ7CleanAir          (27.5) //RS / R0 = 27.5 ppm
#define ADC_Bit_Resolution (12)  // 10 bit ADC
#define Voltage_Resolution (5)   // Volt resolution to calc the voltage
#define Type ("ESP32")           //Board used

//Initialize MQ
MQUnifiedsensor MQ135(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin135, Type);
//MQUnifiedsensor MQ7(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin7, Type);

//WiFi Setting
//WiFiManager wifiManager;
WiFiClient client;

//Thingspeak
unsigned long myChannelNumber = SECRET_CH_ID;
const char *myWriteAPIKey = SECRET_WRITE_APIKEY;
const char *server = "api.thingspeak.com";

//Variable
float co2;
String dataMessage;
String myStatus = "";
int pm03;
int pm25;
int pm10;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(115200);
  pinMode(detectPin, INPUT);
  pmSerial.begin(9600);
  if (!aqi.begin_UART(&pmSerial)) {  // connect to the sensor over software serial
    Serial.println("Could not find PM 2.5 sensor!");
    while (1) delay(10);
  }

  Serial.println("PM25 found!");
  Wire.begin();

  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  //Temperature
  Serial.println("Adafruit AHT10/AHT20 demo!");
  if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  Serial.println("AHT10 or AHT20 found");
  
  //ESP32 Bluetooth
//  SerialBT.begin("ESP32test"); //Bluetooth device name
//  Serial.println("The device started, now you can pair it with bluetooth!");

  // Initialize WiFiManager
  WiFiManager wifiManager;
  //wifiManager.resetSettings();
  wifiManager.autoConnect("Wataverse Technology", "password");

  // Initialize ThingSpeak
  ThingSpeak.begin(client);

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  initSDCard();

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1)
      ;
  }

  //rtc.adjust(DateTime(__DATE__, __TIME__));

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

  //Hidupkan Sensor MQ
  MQ135.init();
  MQ135.setRegressionMethod(1);  //_PPM =  a*ratio^b
  MQ135.setR0(9.03);

  //Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  display.display();
  delay(2);
  display.clearDisplay();


  display.clearDisplay();
  display.setTextColor(WHITE);
  //display.startscrollright(0x00, 0x0F);
  display.setTextSize(2);
  display.setCursor(0, 5);
  display.print("  AQ Meter ");
  display.display();
  delay(3000);
}


void loop() {
  //Temperature and Humidity
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");

  //IR sensor
  detect = digitalRead(detectPin);
  if (detect == 1) {
    Serial.println("Movement detected");
  } else {
    Serial.println("No movement");
  }

  PM25_AQI_Data data;
  if (!aqi.read(&data)) {
    Serial.println("Could not read from AQI");
    delay(500);  // try again in a bit!
    return;
  }
  DateTime now = rtc.now();
  pm03 = data.particles_03um;
  pm25 = data.particles_25um;
  pm10 = data.particles_100um;

  //MQ Sensor
  MQ135.update();
  MQ135.setA(110.47);
  MQ135.setB(-2.862);  //CO2
  float co2 = MQ135.readSensor();

  //Read Date, Time and Sensor
  Serial.print("Date:  ");
  Serial.print(now.day() + String("-") + now.month() + String("-") + now.year());
  Serial.print(", Time:  ");
  Serial.print(String("||") + now.hour() + String(":") + now.minute() + String(":") + now.second());
  Serial.print(", CO2 (PPM):      ");
  Serial.print(pm03);
  Serial.print("CO2:       ");
  Serial.println(co2);
  Serial.print("Motion:       ");
  Serial.println(detect);
  Serial.print("Temperature:       ");
  Serial.println(temp.temperature);
  Serial.print("Humidity:       ");
  Serial.println(humidity.relative_humidity);  
  Serial.println(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.println("--------------------------------------------------------");

  //Concatenate all info separated by commas
  dataMessage = String(now.day()) + String("/") + String(now.month()) + String("/") + String(now.year()) + String("||") +
                String(now.hour()) + String(":") + String(now.minute()) + String(":") + String(now.second()) + String("||") +
                String("=") + String(pm03) + String(";") + String(pm25) + String(";") + String(pm10) + String(";") + String(co2) + String(";") + detect + 
                String(";") + temp.temperature + String(";") + humidity.relative_humidity +"\r\n";
  //dataMessage = String(epochTime) + "," + String(temp) + "," + String(hum) + "," + String(pres) + "\r\n";
  Serial.print("Saving data: ");
  Serial.println(dataMessage);

  /*
  //Bluetooth
  SerialBT.print(temp.temperature);
  SerialBT.print(":");
  SerialBT.println(humidity.relative_humidity);
  SerialBT.print(":");
  SerialBT.println(co2);
  SerialBT.print(":");
  SerialBT.println(detect);
  */

  //Append the data to file
  appendFile(SD, "/data.txt", dataMessage.c_str());

  //Thingspeak
  ThingSpeak.setField(1, pm03);
  ThingSpeak.setField(2, pm25);
  ThingSpeak.setField(3, pm10);
  ThingSpeak.setField(4, co2);
  ThingSpeak.setField(5, detect);
  ThingSpeak.setField(6, temp.temperature);
  ThingSpeak.setField(7, humidity.relative_humidity);

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
  display.setTextSize(2);
  display.setCursor(75, 0);
  display.println(now.second(), DEC);

  display.setTextSize(2);
  display.setCursor(25, 0);
  display.println(":");

  display.setTextSize(2);
  display.setCursor(65, 0);
  display.println(":");

  display.setTextSize(2);
  display.setCursor(40, 0);
  display.println(now.minute(), DEC);

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(now.hour(), DEC);

  display.setTextSize(2);
  display.setCursor(0, 20);
  display.println(now.day(), DEC);

  display.setTextSize(2);
  display.setCursor(25, 20);
  display.println("-");

  display.setTextSize(2);
  display.setCursor(40, 20);
  display.println(now.month(), DEC);

  display.setTextSize(2);
  display.setCursor(55, 20);
  display.println("-");

  display.setTextSize(2);
  display.setCursor(70, 20);
  display.println(now.year(), DEC);

  display.setTextSize(2);
  display.setCursor(0, 40);
  display.print(pm03);

  display.display();
  delay(15000);
}


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
    display.setCursor(70, 40);
    display.print("save");
    display.display();
    ;
  } else {
    Serial.println("Append failed");
    //display.clearDisplay();  
    display.setTextSize(1);
    display.setCursor(70, 40);
    display.print("NOT save");
    display.display();
  }
  file.close();
}
