//#define CAYENNE_DEBUG
#define CAYENNE_PRINT Serial
#include <CayenneMQTTESP32.h>
#include "Adafruit_PM25AQI.h"
#include <SoftwareSerial.h>

// WiFi network info.
char ssid[] = "iot_kdo@unifi";
char wifiPassword[] = "ismailsa2022";

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char username[] = "MQTT_USERNAME";
char password[] = "MQTT_PASSWORD";
char clientID[] = "CLIENT_ID";
SoftwareSerial pmSerial(3, 2);  // change to RXD
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

//Variable
int pm03; // 0.3um/100mL
int pm05; // 0.5um/100mL
int pm25; // 2.5um/100mL
int pm10; // 10um/100mL
String dataMessage;

//Sensor IR
int detectPin = 4;
bool detect = 0;

void setup() {
  Serial.begin(115200);
	Cayenne.begin(username, password, clientID, ssid, wifiPassword);
  while (!Serial) delay(10);
  Serial.println("Adafruit PMSA003I Air Quality Sensor");
  delay(1000);
  pmSerial.begin(9600);

  // There are 3 options for connectivity!
  if (! aqi.begin_UART(&pmSerial)) { // connect to the sensor over software serial 
    Serial.println("Could not find PM 2.5 sensor!");
    while (1) delay(10);
  }

  Serial.println("PM25 found!");
}

void loop() {
	Cayenne.loop();
}

// Default function for sending sensor data at intervals to Cayenne.
// You can also use functions for specific channels, e.g CAYENNE_OUT(1) for sending channel 1 data.
CAYENNE_OUT_DEFAULT()
{
  //IR
  detect = digitalRead(detectPin);
  
  //Dust Sensor
  delay(1000);
  PM25_AQI_Data data;
  if (!aqi.read(&data)) {
    Serial.println("Could not read from AQI");
    delay(500);  // try again in a bit!
    return;
  }

  //Definition
  pm03 = data.particles_03um;
  pm05 = data.particles_05um;
  pm25 = data.particles_25um;
  pm10 = data.particles_100um;

  //Concatenate all info separated by commas
  dataMessage = String("|") + pm03 + String(";") + pm05 + String(";") + pm25 + String(";") + pm10 + String(";") + detect + "\r\n";
  Serial.print("Saving data: ");
  Serial.println(dataMessage);
  
	// Write data to Cayenne here. This example just sends the current uptime in milliseconds on virtual channel 0.
	Cayenne.virtualWrite(0, millis());
  Cayenne.virtualWrite(1, pm03);
  Cayenne.virtualWrite(2, pm05);
  Cayenne.virtualWrite(3, pm25);
  Cayenne.virtualWrite(4, pm10);
  Cayenne.virtualWrite(5, detect);
  delay(5000);
}

// Default function for processing actuator commands from the Cayenne Dashboard.
// You can also use functions for specific channels, e.g CAYENNE_IN(1) for channel 1 commands.
CAYENNE_IN_DEFAULT()
{
	CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
	//Process message here. If there is an error set an error message using getValue.setError(), e.g getValue.setError("Error message");
}
