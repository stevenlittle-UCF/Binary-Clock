// Writen by Steven Little
// Binary clock that uses 3 shift registers and a NodeMCU with ESP8288

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <time.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

// name them so code is readable
#define data1 D0
#define clock1 D1
#define latch1 D2
#define data2 D3
#define clock2 D4
#define latch2 D5
#define data3 D6
#define clock3 D7
#define latch3 D8

bool lights = true;
bool ota_flag = false;
uint16_t time_elapsed = 0;

// wifi info
const char* ssid = "EnterSSID";
const char* password = "EnterPASS";

// place we go for information
const char* url = "http://ENTERURL/time";

// declaration of HTTP server
ESP8266WebServer server(80);

// start serial monitor and wifi
void setup()
{
  pinMode(latch1, OUTPUT);
  pinMode(data1, OUTPUT);  
  pinMode(clock1, OUTPUT);
  pinMode(latch2, OUTPUT);
  pinMode(data2, OUTPUT);  
  pinMode(clock2, OUTPUT);
  pinMode(data3, OUTPUT);
  pinMode(clock3, OUTPUT);
  pinMode(latch3, OUTPUT);
  
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting..");
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
   ArduinoOTA.setHostname("Clock-ESP8266");

  // No authentication by default
  ArduinoOTA.setPassword("password");

  ArduinoOTA.onStart([]() 
  {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      type = "sketch";
    }
    else
    { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]()
  {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
  {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error)
  {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
      Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
      Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
      Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR)
      Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  Serial.print("\nConnected with ip: \t");
  Serial.println(WiFi.localIP());

  // Mange the server
  server.on("/toggle", HTTP_POST, toggleLight);
  server.on("/program", HTTP_POST, startProgram);
  server.on("/", HTTP_GET, homepage);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop()
{
  server.handleClient();
  
  if (ota_flag)
  {
    uint16_t time_start = millis();
    while (time_elapsed < 10000)
    {
      ArduinoOTA.handle();
      time_elapsed = millis() - time_start;
      delay(10);
    }

    ota_flag = false;
  }
  
  if (WiFi.status() == WL_CONNECTED && lights)
  {
    HTTPClient http;

    http.begin(url);

    int responseCode = http.GET();

    if (responseCode == 200)
    {
      const size_t capacity = JSON_OBJECT_SIZE(3) + 30;
      DynamicJsonDocument doc(capacity);
      
      deserializeJson(doc, http.getString());
      
      int hr = doc["hour"]; // 12
      int mins = doc["minute"]; // 41
      int sec = doc["second"]; // 42

      displayTime(hr, mins, sec);
    }
    else
    {
      displayTime(0,0,0);
    }
  }
  else
  {
    displayTime(0,0,0);
  }

  delay(100);
}

void displayTime(int hr, int mins, int sec)
{
//      Serial.println(sec);
  byte newHour = hr % 10;
  hr = ((hr / 10) * 16) + newHour;

  byte newMins = mins % 10;
  mins = ((mins / 10) * 16) + newMins;

  byte newSec = sec % 10;
  sec = ((sec / 10) * 16) + newSec;

  shiftSec(sec);
  shiftMin(mins);
  shiftHr(hr);
}

void shiftSec(byte secs)
{
  digitalWrite(latch1, LOW);
  shiftOut(data1, clock1, MSBFIRST, secs);
  digitalWrite(latch1, HIGH);
}

void shiftMin(byte mins)
{
  digitalWrite(latch2, LOW);
  shiftOut(data2, clock2, MSBFIRST, mins);
  digitalWrite(latch2, HIGH);
}

void shiftHr(byte hrs)
{
  digitalWrite(latch3, LOW);
  shiftOut(data3, clock3, MSBFIRST, hrs);
  digitalWrite(latch3, HIGH);
}

// This toggles if the LEDS can turn on
void toggleLight()
{
  lights = !lights;
  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/plain", "The lights have been toggled!");
}

// This handles a page not found on the server
void handleNotFound()
{
  server.send(404, "text/plain", "404: Not found");
}

void homepage()
{
  server.send(200, "text/html","<h1>The current light status is:</h1><h2>" + getStatus() + "</h2><h3>" + getProgramStatus() + "</h3><form action=\"/toggle\" method=\"post\"><button type=\"submit\">Toggle lights</button></form><form action=\"/program\" method=\"post\"><button type =\"submit\">Toggle Programming mode</button>");
}

void startProgram()
{
  displayTime(11,17,57);
  ota_flag = true;
  time_elapsed = 0;
  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/plain", "Programming mode toggled!");
}

String getStatus()
{
  if (!lights)
    return "OFF!";
   else
    return "ON!";
}

String getProgramStatus()
{
  if (ota_flag)
    return "The device is currently in programming mode!";
  else
    return "The device is not currently in programming mode!";
}
