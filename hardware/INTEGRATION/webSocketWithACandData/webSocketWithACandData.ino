#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <AutoConnect.h>
 
WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266WebServer server(80);   //instantiate server at port 80 (http port)
AutoConnect      Portal(server);
 
String page = "";
const int LED_PIN = 0;
const int ANALOG_PIN = 17;
int sensorData, data;
String data_str;

//sample timing variables
unsigned long previousMillis = 0;
unsigned long currentMillis;
const long INTERVAL = 1000;
 
//additional attibutes
uint8_t socketNumber;

void setup(){
 //the HTML of the web page
  page = "<h1>Sensor to Node MCU Web Server</h1><h3>Force: "+ String(getData()) + "</h3>";
  //page = "<h1>Sensor to Node MCU Web Server</h1><h3>Force: "+String(data)+"</h3>";
 //pin assignments
 pinMode(ANALOG_PIN, INPUT);
 pinMode(LED_PIN, OUTPUT);
 //connection status LED off
 digitalWrite(LED_PIN, HIGH);
 
 delay(1000);
 Serial.begin(115200);
 Serial.println();

 server.on("/", [](){
    server.send(200, "text/html", page);
 });
 
 Portal.begin();
 webSocket.begin();
 webSocket.onEvent(webSocketEvent);
 //connection status message
 Serial.println("Web server started!");
 //connection status LED
 digitalWrite(LED_PIN, LOW);
}


void loop(){

  currentMillis = millis();
  if(currentMillis - previousMillis >= INTERVAL) {
    previousMillis = currentMillis;
    sensorData = analogRead(ANALOG_PIN);
    data = map(sensorData,0,1024,0,45);
    data_str = String(data);

    webSocket.broadcastTXT(data_str);
 
    Serial.print("sensorData = "); Serial.println(sensorData);
    Serial.print("mapData = "); Serial.println(data);
    Serial.println("");
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

int getData(){
  return data;
}
