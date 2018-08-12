#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <AutoConnect.h>
 
//// Replace with your network credentials
//const char* ssid = "KBMOTO2";
//const char* password = "Etn!esB0nesNyr@";
 
WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266WebServer server(80);   //instantiate server at port 80 (http port)
AutoConnect      Portal(server);
 
String page = "";
int LEDPin = 16;
 
void setup(){
 //the HTML of the web page
 page = "<h1>Simple NodeMCU Web Server</h1><p><a href=\"LEDOn\"><button>ON</button></a>&nbsp;<a href=\"LEDOff\"><button>OFF</button></a></p>";
 //make the LED pin output and initially turned off
 pinMode(LEDPin, OUTPUT);
 digitalWrite(LEDPin, HIGH);
 
 delay(1000);
 Serial.begin(115200);
 Serial.println();

 server.on("/", [](){
    server.send(200, "text/html", page);
 });
 
 server.on("/LEDOn", [](){
    server.send(200, "text/html", page);
    digitalWrite(LEDPin, LOW);
    delay(1000);
 });
 
 server.on("/LEDOff", [](){
    server.send(200, "text/html", page);
    digitalWrite(LEDPin, HIGH);
    delay(1000);
 });
 
 Portal.begin();
 webSocket.begin();
 webSocket.onEvent(webSocketEvent);
 
 Serial.println("Web server started!");
}
 
void loop(){
  webSocket.loop();
  Portal.handleClient();
  if (Serial.available() > 0){
    char c[] = {(char)Serial.read()};
    webSocket.broadcastTXT(c, sizeof(c));
  }
}
 
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
  if (type == WStype_TEXT){
   for(int i = 0; i < length; i++) Serial.print((char) payload[i]);
   Serial.println();
  }
}
