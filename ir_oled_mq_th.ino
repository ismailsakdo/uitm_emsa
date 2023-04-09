#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MQUnifiedsensor.h>
#include <SHT21.h>  // include SHT21 library

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
    Serial.print("Humidity:");
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
    Serial.print("Humidity:");
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
  delay(500);
}
