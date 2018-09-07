/*
    Simple HTTP get webclient test
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#define USE_SERIAL Serial
#include <WiFiClient.h>
#define FASTLED_ALLOW_INTERRUPTS 0
#include "FastLED.h"

// WiFi Stuff
const char* ssid     = "<SSID>";
const char* password = "<PASSWORD>";
WiFiClient client;

String myMode = "startup";
String currentMode;

// LED Controller API
const char* host_address = "192.168.1.3";
const int host_port = 4444;

const String deviceName = "ledstrip1";
String url = "/v1/devices/" + deviceName;

// LED Stuff

#define NUM_LEDS    300
#define DATA_PIN    15
#define LED_TYPE    WS2812B

#define RED   0
#define GREEN 0
#define BLUE  0


// Define the array of leds
CRGB leds[NUM_LEDS];


unsigned long start, finish;

void setup() {
  // Wifi Stuff
  Serial.begin(115200);
  delay(100);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(5000);

  Serial.print("connecting to ");
  Serial.println(host_address);


  // Use WiFiClient class to create TCP connections
  if (!client.connect(host_address, host_port)) {
    Serial.println("connection failed");
  }

  // We now create a URI for the request
  // This will send the request to the server
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host_address + ":" + host_port + "\r\n" +
               "Connection: Keep-Alive\r\n\r\n");
  delay(1000);

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  //LED Stuff
  FastLED.addLeds<LED_TYPE, DATA_PIN, RGB>(leds, NUM_LEDS);
  LEDS.setBrightness(64);

  //On board LED
  pinMode(0, OUTPUT);

}

void loop() {
  checkMode();
  myMode.trim();
  while (myMode == "startup") {
    currentMode = myMode;
  }
  while (myMode == "0") {
    // LED Off
    ledOff();
    currentMode = myMode;
    checkMode();
    myMode.trim();
  }
  while (myMode == "1") {
    cylon();
    currentMode = myMode;
    checkMode();
    myMode.trim();
  }
  while (myMode == "2") {
    partyMode();
    currentMode = myMode;
    checkMode();
    myMode.trim();
  }
  while (myMode != "startup" || myMode != "1" || myMode != "2" || myMode != "1") {
    // Set color
    checkMode();
    myMode.trim();
    if (myMode == "startup" || myMode == "0" || myMode == "1" || myMode == "2") {
      break;
    }
    staticColor(myMode);
    currentMode = myMode;
  }
}
// End main loop



void checkMode ( )
{

  client.setTimeout(100);
  Serial.print("Checking Mode.");

  // This will send the request to the server
  client.setNoDelay(true);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host_address + ":" + host_port + "\r\n" +
               "Connection: Keep-Alive\r\n\r\n");

  delay(100);
  // Read all the lines of the reply from server and print them to Serial
  start = millis ();
  while (client.available()) {
    String line = client.readStringUntil('\r\n');
    myMode = line;
  }
  finish = millis ();
  Serial.print ("Time taken: ");
  Serial.println (finish - start);
  Serial.println("\n\nIn Check, myMode: " + myMode + "\n\n");
}

void taskOnBoard ( void )
{
  digitalWrite(0, LOW);
  Serial.println("LED ON");
}

void ledOff () {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
    FastLED.show();
    fadeall();
    delay(10);
  }
}

void fadeall() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(250);
  }
}

void cylon() {
  static uint8_t hue = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(hue++, 255, 255);
    FastLED.show();
    fadeall();
    delay(5);
  }
  checkMode();
  myMode.trim();
  if ( myMode != "1" ) {
    return;
  }
  // Now go in the other direction.
  for (int i = (NUM_LEDS) - 1; i >= 0; i--) {
    leds[i] = CHSV(hue++, 255, 255);
    FastLED.show();
    fadeall();
    delay(5);
  }
}

void staticColor(const String& rgbValue) {
  Serial.println("In Static Color.");
  Serial.println("RGB Value: " + rgbValue);
  int dashIndex = rgbValue.indexOf('-');
  int secondDashIndex = rgbValue.indexOf('-', dashIndex + 1);
  int thirdDashIndex = rgbValue.indexOf('-', secondDashIndex + 1);
  String firstValue = rgbValue.substring(0, dashIndex);
  String secondValue = rgbValue.substring(dashIndex + 1, secondDashIndex);
  String thirdValue = rgbValue.substring(secondDashIndex + 1);
  String fourthValue = rgbValue.substring(thirdDashIndex + 1);
  int g = firstValue.toInt();
  int r = secondValue.toInt();
  int b = thirdValue.toInt();
  int bright = fourthValue.toInt();

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].r = r;
    leds[i].g = g;
    leds[i].b = b;
    FastLED.setBrightness(bright);
    FastLED.show();
    delay(2);
  }

}

void partyMode() {
  int r = random(0, 255);
  int g = random(0, 255);
  int b = random(0, 255);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].r = r;
    leds[i].g = g;
    leds[i].b = b;
//    FastLED.setBrightness(bright);
    FastLED.show();
    delay(2);
  }
}

