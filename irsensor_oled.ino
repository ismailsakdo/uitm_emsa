
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Sensor IR
int detectPin = 4;
bool detect = 0;

//option display
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     // Reset pin # (or -1 if sharing reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  pinMode(detectPin, INPUT);
  Wire.begin();

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
  display.print("  IR Meter ");
  display.display();
  delay(3000);
  display.clearDisplay();
}


void loop() {
  //IR sensor
  detect = digitalRead(detectPin);
  if (detect != 1) {
    Serial.println("Movement detected");
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Detected");

  } else {
    Serial.println("No movement");
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("NOT Detected");
  }

  display.display();
  delay(500);
}
