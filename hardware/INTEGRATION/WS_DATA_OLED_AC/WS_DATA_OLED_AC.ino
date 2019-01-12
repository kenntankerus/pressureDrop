#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WebSocketsServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiManager.h>

/* Set these to your desired credentials. */
const char WiFiAPPSK[] = "";

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
   
#define ANALOG_PIN  17
#define BUTTON_A    0
#define BUTTON_B    16
#define BUTTON_C    2
#define USE_SERIAL Serial
#define DBG_OUTPUT_PORT Serial

int sensorData;
int force = 0;
int oldForce = 0;

uint8_t remote_ip;
uint8_t socketNumber;

String x = "";

ESP8266WebServer server(80);

// Create a Websocket server
WebSocketsServer webSocket(81);

// state machine states
unsigned int state;
#define SEQUENCE_IDLE 0x00
#define GET_SAMPLE 0x10
#define GET_SAMPLE__WAITING 0x12

void setup() {
  delay(1000);
  Serial.begin(9600);

  pinMode(ANALOG_PIN,INPUT);
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen. 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  display.setTextColor(WHITE); display.setTextSize(1); display.display();
  delay(2000);
 
  SPIFFS.begin();
  Serial.println();
  Serial.printf("\nConfiguring WiFi\nPlease connect to:\n  AutoConnectAP");
  display.clearDisplay(); display.display(); display.setCursor(0,0);
  display.println("Configuring WiFi\nPlease connect to:\n  AutoConnectAP"); display.display(); 

  setupWiFi();
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  
  server.on("/", HTTP_GET, []() {
    handleFileRead("/");
  });

  //Handle when user requests a file that does not exist
  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
    server.send(404, "text/plain", "FileNotFound");
  });

  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  server.begin();
  
  display.clearDisplay(); display.display();
  display.setTextSize(2); display.setCursor(0,0); 
  display.println("Web server started!"); display.display(); delay(2000);

  // initialize sensor OLED display
  display.clearDisplay(); display.setCursor(0,0);
  display.println("Force:"); display.print(oldForce); display.print("  Newtons");
  display.display();
}

void loop() {
  webSocket.loop();
  analogSample();
  server.handleClient();
  handleButtons();
}

void analogSample(void) {
  if (state == SEQUENCE_IDLE) {
    return;
  }
  else if (state == GET_SAMPLE) {
    state = GET_SAMPLE__WAITING;
    return;
  }
  else if (state == GET_SAMPLE__WAITING) {
    sensorData = analogRead(ANALOG_PIN);
    force = map(sensorData,0,1024,0,45);
    if(force != oldForce) {
      display.setCursor(0,16); display.setTextColor(BLACK);
      display.print(oldForce); display.display();
      display.setCursor(0,16); display.setTextColor(WHITE);
      display.print(force); display.display();
      oldForce = force;
      Serial.printf("Sensor Data: %u of 1024\n", sensorData);
      Serial.printf("Force: %u Newtons \n\n", force);  
    }
    String force_str = String(force);
    webSocket.sendTXT(socketNumber, "wpMeter,Arduino," + force_str + ",1");
    //Delay sending next sample so that the web server can respond
    delay(50);
    return;
  }
}

void handleButtons(void) {
  if (!digitalRead(BUTTON_A)) {
    while (!digitalRead(BUTTON_A)) {
      display.clearDisplay(); display.setTextSize(1); display.setCursor(0, 0);
      display.print("SSID:\n  "); display.println(WiFi.SSID());
      display.print("IP Address:\n  "); display.println(WiFi.localIP());
      display.display();
    }
    display.clearDisplay(); display.setTextSize(2); display.setCursor(0, 0);
    display.println("Force:"); display.print(oldForce);
    display.print("  Newtons"); display.display();  
  }
  
  if (!digitalRead(BUTTON_B))
    ;
  if (!digitalRead(BUTTON_C)) {
    int counter = 9;
    display.clearDisplay(); display.setTextSize(1); display.setCursor(0, 8);
    display.print("HOLD TO CLEAR \nWIFI SETTINGS"); 
    display.setCursor(100, 6); display.setTextSize(3); 
    display.print(counter); display.display();
    while (!digitalRead(BUTTON_C)) {
      counter = counter - 1;
      if (counter == -1) {
        display.clearDisplay(); display.setTextSize(1); display.setCursor(0,0);
        display.println("WiFi settings cleared\nDevice will restart\nPlease connect to:\n  AutoConnectAP");
        display.display();
        WiFiManager wifiManager; wifiManager.resetSettings(); delay(4000);
        ESP.restart();
      }
      delay(1000);  
      display.setCursor(100, 6); display.setTextColor(BLACK);
      display.print(counter + 1); display.display();
      display.setCursor(100, 6); display.setTextColor(WHITE);
      display.print(counter); display.display();
    }
    display.clearDisplay(); display.setTextSize(2); display.setCursor(0, 0);
    display.println("Force:"); display.print(oldForce);
    display.print("  Newtons"); display.display(); 
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t lenght) {
  switch (type) {
    case WStype_DISCONNECTED: {
      //Reset the control for sending samples of ADC to idle to allow for web server to respond.
      USE_SERIAL.printf("[%u] Disconnected!\n", num);
      state = SEQUENCE_IDLE;            
      } break;
    case WStype_CONNECTED: {
      //Display client IP info that is connected in Serial monitor and set control to enable samples to be sent every two seconds (see analogsample() function)
      IPAddress ip = webSocket.remoteIP(num);
      USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      socketNumber = num;
      state = GET_SAMPLE;               
      } break;
    case WStype_TEXT: {
        if (payload[0] == '#') {
            Serial.printf("[%u] get Text: %s\n", num, payload);
        }                               
        } break;
    case WStype_ERROR:
        USE_SERIAL.printf("Error [%u] , %s\n", num, payload);
    }
}

String getContentType(String filename) {
  if (server.hasArg("download"))
    return "application/octet-stream";
  else if (filename.endsWith(".htm"))
    return "text/html";
  else if (filename.endsWith(".html"))
    return "text/html";
  else if (filename.endsWith(".css"))
    return "text/css";
  else if (filename.endsWith(".js"))
    return "application/javascript";
  else if (filename.endsWith(".png"))
    return "image/png";
  else if (filename.endsWith(".gif"))
    return "image/gif";
  else if (filename.endsWith(".jpg"))
    return "image/jpeg";
  else if (filename.endsWith(".ico"))
    return "image/x-icon";
  else if (filename.endsWith(".xml"))
    return "text/xml";
  else if (filename.endsWith(".pdf"))
    return "application/x-pdf";
  else if (filename.endsWith(".zip"))
    return "application/x-zip";
  else if (filename.endsWith(".gz"))
    return "application/x-gzip";
  else if (filename.endsWith(".svg"))
    return "image/svg+xml";
  return "text/plain";
}

bool handleFileRead(String path) {
  DBG_OUTPUT_PORT.println("handleFileRead: " + path);
  if (path.endsWith("/")) {
    path += "counter.html";
    state = SEQUENCE_IDLE;
    }
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  DBG_OUTPUT_PORT.println("PathFile: " + pathWithGz);
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void setupWiFi() {
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //exit after config instead of connecting
  wifiManager.setBreakAfterConfig(true);

  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP" with password "password"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
    display.clearDisplay(); display.setTextSize(1); display.setCursor(0, 0);
    display.printf("/nfailed to connect/n retrying..."); display.display();
    
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000); ESP.reset(); delay(5000);
  }

  //if you get here you have connected to the WiFi
  display.clearDisplay(); display.setTextSize(1); display.setCursor(0, 0);
  display.print("connected to:\n  "); display.println(WiFi.SSID());
  display.print("IP Address:\n  "); display.println(WiFi.localIP());
  display.display();
  Serial.println("wifi connected...");
  Serial.print("network ID:\n  "); Serial.println(WiFi.SSID());
  Serial.print("local ip:\n  "); ; Serial.println(WiFi.localIP()); delay(2000);

}
