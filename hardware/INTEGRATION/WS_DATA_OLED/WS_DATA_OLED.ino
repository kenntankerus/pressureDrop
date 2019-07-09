#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <WebSocketsServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  display.setTextColor(WHITE); display.setTextSize(1);
  display.display(); delay(1000);
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  
  SPIFFS.begin();
  Serial.println();
  Serial.printf("\nConfiguring access point...\n");
  display.clearDisplay(); display.display(); display.setCursor(0,0);
  display.println("Configuring\naccess point..."); display.display(); 
  
  /* You can remove the password parameter if you want the AP to be open. */
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
  display.clearDisplay(); display.display(); display.setCursor(0,0);
  display.println("Force:");
  display.print(oldForce);
  display.print("  Newtons");
  display.display();
}

void loop() {
  webSocket.loop();
  analogSample();
  server.handleClient();
  handleButtons();
}

/* Go to http://192.168.4.1 in a web browser
* connected to this access point to see it. 
*/

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
  if (!digitalRead(BUTTON_C)) 
    ;
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
  WiFi.mode(WIFI_AP);
  String AP_NameString = "ForceSensor";
  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i = 0; i < AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);
  WiFi.softAP(AP_NameChar, WiFiAPPSK);
}
