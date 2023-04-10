#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MQUnifiedsensor.h>
#include <Adafruit_BMP280.h>

//Sensor IR
int detectPin = 4;
bool detect = 0;

//temp&humid
Adafruit_BMP280 bmp; // I2C

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

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(115200);
  Wire.begin();

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


  //temp & pressure
  temp = bmp.readTemperature();  // get temp from SHT
  pressure = bmp.readPressure(); // get temp from SHT

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
    Serial.print("Pressure:");
    Serial.println(pressure);
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
    display.println(pressure);

  } else {
    Serial.print("No movement ");
    Serial.println(co2);
    Serial.print(" Temperature:");
    Serial.print(temp);
    Serial.print("Pressure:");
    Serial.println(pressure);
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
    display.println(pressure);
  }

  display.display();
  delay(500);
}
