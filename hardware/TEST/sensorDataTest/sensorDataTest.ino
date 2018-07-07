#include <Arduino.h>
const int NUM_SAMPLES = 10;
const int ANALOG_PIN = 17;
int sensorData, sensorForce;
int sample[NUM_SAMPLES];
int sampleCount = 0;

unsigned long previousMillis = 0;
unsigned long currentMillis;
const long interval = 1000;

void setup() {
    Serial.begin(115200);
    pinMode(ANALOG_PIN, INPUT);
}

void loop() { 
    currentMillis = millis();
    if(currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        sensorData = analogRead(ANALOG_PIN);
        sample[sampleCount] = map(sensorData,313,595,0,200);
        Serial.print("Force = ");
        Serial.print(sample[sampleCount]);
        Serial.println(" Newtons");
        sampleCount++;
    }
    if(sampleCount >= NUM_SAMPLES) {
        return;
    }
}