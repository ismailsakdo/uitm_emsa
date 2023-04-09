#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MQUnifiedsensor.h>
#include <SHT21.h>  // include SHT21 library

//Wifi
#define SECRET_SSID "username"    // replace MySSID with your WiFi network name
#define SECRET_PASS "password"  // replace MyPassword with your WiFi password
#define SECRET_CH_ID 00000     // replace 0000000 with your channel number
#define SECRET_WRITE_APIKEY "WRITE_API_KEY"   // replace XYZ with your channel write API Key

//Wifi Setup thingspeak
#include <WiFi.h>
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros
char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;
unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
String myStatus = "";

//Sensor IR
int detectPin = 4;
bool detect = 0;

//temp&humid
SHT21 sht;

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
float humidity;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(115200);
  Wire.begin();

  //Hidupkan Sensor MQ
  MQ135.init();
  MQ135.setRegressionMethod(1);  //_PPM =  a*ratio^b
  MQ135.setR0(9.03);

  //WiFi
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);  // Initialize ThingSpeak

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
  //internet stuff
  internet();
  
  //temp & humid
  temp = sht.getTemperature();  // get temp from SHT
  humidity = sht.getHumidity(); // get temp from SHT

  //MQ Sensor
  MQ135.update();
  MQ135.setA(110.47);
  MQ135.setB(-2.862);  //CO2
  float co2 = MQ135.readSensor();

  //IR sensor
  detect = digitalRead(detectPin);
  if (detect != 1) {
    Serial.print("Movement detected ");
    Serial.print(co2);
    Serial.print(" Temperature:");
    Serial.print(temp);
    Serial.print(" Humidity:");
    Serial.println(humidity);
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Detected");
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.println(co2);
    display.setTextSize(1);
    display.setCursor(0, 30);
    display.println(temp);
    display.setTextSize(1);
    display.setCursor(0, 40);
    display.println(humidity);

  } else {
    Serial.print("No movement ");
    Serial.println(co2);
    Serial.print(" Temperature:");
    Serial.print(temp);
    Serial.print(" Humidity:");
    Serial.println(humidity);
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("NOT Detected");
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.println(co2);
    display.setTextSize(1);
    display.setCursor(0, 30);
    display.println(temp);
    display.setTextSize(1);
    display.setCursor(0, 40);
    display.println(humidity);
  }

  display.display();
  internet();
  delay(15000);
}

void internet(){  
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

  // set the fields with the values
  ThingSpeak.setField(1, co2);
  ThingSpeak.setField(2, temp);
  ThingSpeak.setField(3, humidity);
  ThingSpeak.setField(4, detect);

    // set the status
  ThingSpeak.setStatus(myStatus);
  
  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
}
