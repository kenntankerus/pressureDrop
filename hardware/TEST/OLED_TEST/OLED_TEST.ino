#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

#define ANALOG_PIN 17
#define BUTTON_A 0
#define BUTTON_B 16
#define BUTTON_C 2

int sensorData, force;
int oldForce = 0;

void setup()
{
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  pinMode(ANALOG_PIN, INPUT);
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Force:");
  display.print(oldForce);
  display.print("  Newtons");
  display.display();
}

void loop()
{

  sensorData = analogRead(ANALOG_PIN);
  force = map(sensorData, 0, 1024, 0, 45);

  if (force != oldForce)
  {
    display.setCursor(0, 16);
    display.setTextColor(BLACK);
    display.print(oldForce);
    display.display();
    display.setCursor(0, 16);
    display.setTextColor(WHITE);
    display.print(force);
    display.display();
    oldForce = force;

    Serial.printf("Sensor Data: [%u]\n", sensorData);
    Serial.printf("Force: [%u]\n\n", force);
  }

  if (!digitalRead(BUTTON_A))
  {
    while (!digitalRead(BUTTON_A))
    {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("SSID:\n  ");
      display.println("local wireless");
      display.print("IP ADDRESS:\n  ");
      display.println("192.168.1.13");
      display.display();
    }
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("Force:");
    display.print(oldForce);
    display.print("  Newtons");
    display.display();
  }

  if (!digitalRead(BUTTON_B))
    ;
  if (!digitalRead(BUTTON_C))
    ;
  yield();
}
