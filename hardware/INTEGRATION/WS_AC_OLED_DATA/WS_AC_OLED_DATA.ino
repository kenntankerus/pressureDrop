#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <AutoConnect.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266WebServer server(80);   //instantiate server at port 80 (http port)
AutoConnect      Portal(server);     
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
   
#define ANALOG_PIN  17
#define BUTTON_A    0
#define BUTTON_B    16
#define BUTTON_C    2

String page = "";
String force_str;
int sensorData;
int force = 0;
int oldForce = 0;

//sample timing variables
unsigned long previousMillis = 0;
unsigned long currentMillis;
const long INTERVAL = 1000;
 
//additional attibutes
uint8_t socketNumber;

void setup() {
  Serial.begin(9600);
  //the HTML of the web page
  page = "<h1>Sensor to Node MCU Web Server</h1><h3>Force: "+ String(getForce()) + "</h3>";
  
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
  
  pinMode(ANALOG_PIN,INPUT);
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
     
  Serial.println();
  
  server.on("/", [](){
    server.send(200, "text/html", page);
  });

  Portal.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  //connection status message
  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Web server started!");
  display.display(); delay(2000);
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Force:");
  display.print(oldForce);
  display.print("  Newtons");
  display.display();
}

void loop() {
  sensorData = analogRead(ANALOG_PIN);
  force = map(sensorData,0,1024,0,45);
  if(force != oldForce) {
    display.setCursor(0,16);
    display.setTextColor(BLACK);
    display.print(oldForce);
    display.display();
    display.setCursor(0,16);
    display.setTextColor(WHITE);
    display.print(force);
    display.display();
    oldForce = force;

    Serial.printf("Sensor Data: %u of 1024\n", sensorData);
    Serial.printf("Force: %u Newtons \n\n", force);  
  }

   
  if(!digitalRead(BUTTON_A)) {
    while(!digitalRead(BUTTON_A)) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0,0);
      display.print("SSID:\n  ");
      display.println("local wireless");
      display.print("IP ADDRESS:\n  ");
      display.println("192.168.1.13");
      display.display();         
    }
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0,0);
    display.println("Force:");
    display.print(oldForce);
    display.print("  Newtons");
    display.display();
  }
      
  if(!digitalRead(BUTTON_B));
  if(!digitalRead(BUTTON_C));
  yield();

  currentMillis = millis();
  if(currentMillis - previousMillis >= INTERVAL) {
    previousMillis = currentMillis;
    force_str = String(force);

    webSocket.broadcastTXT(force_str);
 
    Serial.printf("Sensor Data: %u of 1024\n", sensorData);
    Serial.printf("Force: %u Newtons \n\n", force); 
  }
  webSocket.loop();
  Portal.handleClient();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  switch (type) { 
    case WStype_DISCONNECTED:
      // Reset the control for sending samples of ADC to idle to allow for web server to respond.
      Serial.printf("[%u] Disconnected!\n", num);
      yield();
      break;
    case WStype_CONNECTED: {                  // Braces required http://stackoverflow.com/questions/5685471/error-jump-to-case-label
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      yield();
      socketNumber = num;
      break;
      }
    case WStype_TEXT:
      if (payload[0] == '#') {
        Serial.printf("[%u] get Text: %s\n", num, payload);
        yield();
      }
      break;
    case WStype_ERROR:
      Serial.printf("Error [%u] , %s\n", num, payload);
      yield();     
  }
}

int getForce(){
  return force;
}
