#include <AutoConnect.h>
#include <AutoConnectCredential.h>
#include <AutoConnectPage.h>

#include <Arduino.h>
int r;

void setup() {
  Serial.begin(115200);
  r = 0;
}

void loop() { 
  Serial.println(r++);
  delay(1000);
}
