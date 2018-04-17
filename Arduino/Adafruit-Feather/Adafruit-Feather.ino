/*
 *  Simple HTTP get webclient test
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#define USE_SERIAL Serial

#include "FastLED.h"

// WiFi Stuff
const char* ssid     = "<SSID>";
const char* password = "<PASSWORD>";

// LED Controller API
const char* host_address = "192.168.1.3";
const int host_port = 4444;

const String deviceName = "ledstrip1";

// LED Stuff

#define NUM_LEDS    300
#define DATA_PIN    15
#define LED_TYPE    WS2812B

#define RED   0
#define GREEN 0
#define BLUE  0


// Define the array of leds
CRGB leds[NUM_LEDS];

typedef struct Timer
{
   unsigned long start;
   unsigned long timeout;
};

char TimerExpired ( struct Timer * timer )
{
   if ( millis () > timer->start + timer->timeout )
       return true;

   return false;
}

void TimerStart ( struct Timer * timer )
{
   timer->start = millis ( );
}

//wifi task running every 3000 milliseconds
Timer timerWiFi = { 0, 500 };
//onboard task running every 10000 milliseconds
Timer timerLED = { 0, 100 };

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
  WiFiClient client;
  if (!client.connect(host_address, host_port)) {
    Serial.println("connection failed");
  }

  // We now create a URI for the request
  String url = "/v1/devices/" + deviceName;
  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host_address +":"+host_port + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(500);

  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  //LED Stuff
  FastLED.addLeds<LED_TYPE, DATA_PIN, RGB>(leds, NUM_LEDS);
  LEDS.setBrightness(64);

  //On board LED
  pinMode(0, OUTPUT);

}

int value = 0;
String myMode = "startup";
String currentMode;
void loop() {

     if ( TimerExpired ( & timerWiFi ) )
   {
      currentMode = myMode;
      Serial.println("Current Mode: " + currentMode);
      Serial.println("myMode: " + myMode);
       checkMode(myMode);
       myMode.trim();

       TimerStart ( & timerWiFi );
   }
        if (myMode == "0"){
          // LED Off
          ledOff();
          currentMode = myMode;
        }
        else if (myMode == "1"){
          cylon();
          currentMode = myMode;
        }
        else if (myMode == "2"){

          currentMode = myMode;
        }
        else if (myMode == "3"){

          currentMode = myMode;
        }
        else if (myMode == "startup"){

          currentMode = myMode;
        }
        else {
          // Set color
          staticColor(myMode);
          currentMode = myMode;
        }
}
// End main loop



void checkMode (String &myMode )
{
  ++value;

  Serial.print("connecting to ");
  Serial.println(host_address);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host_address, host_port)) {
    Serial.println("connection failed");
  }

  String url = "/v1/devices/" + deviceName;
  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host_address + ":" + host_port + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(500);

  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    myMode = line;
  }

  Serial.println();
  Serial.println("closing connection");
}

void taskOnBoard ( void )
{

  digitalWrite(0, LOW);
  Serial.println("LED ON");

}

void ledOff (){
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
    FastLED.show();
    fadeall();
    delay(10);
  }

}

void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } }

void cylon(){
  static uint8_t hue = 0;
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(hue++, 255, 255);
    FastLED.show();
    fadeall();
    delay(10);
  }
  // Now go in the other direction.
  for(int i = (NUM_LEDS)-1; i >= 0; i--) {
    leds[i] = CHSV(hue++, 255, 255);
    FastLED.show();
    fadeall();
    delay(10);
  }
}

void staticColor(const String& rgbValue){
  Serial.println("In Static Color.");
  Serial.println("RGB Value: " + rgbValue);
  int dashIndex = rgbValue.indexOf('-');
  int secondDashIndex = rgbValue.indexOf('-', dashIndex + 1);
  int thirdDashIndex = rgbValue.indexOf('-',secondDashIndex + 1);
  String firstValue = rgbValue.substring(0, dashIndex);
  String secondValue = rgbValue.substring(dashIndex + 1, secondDashIndex);
  String thirdValue = rgbValue.substring(secondDashIndex + 1);
  String fourthValue = rgbValue.substring(thirdDashIndex + 1);
  int g = firstValue.toInt();
  int r = secondValue.toInt();
  int b = thirdValue.toInt();
  int bright = fourthValue.toInt();
  Serial.println("RGB:");
  Serial.println("r: " + r);
  Serial.println("g: " + g);
  Serial.println("b: " + b);
  Serial.println("Brightness: " + bright);

  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i].r = r;
    leds[i].g = g;
    leds[i].b = b;
    FastLED.setBrightness(bright);
    FastLED.show();
    delay(2);
  }

}
