#include <Adafruit_HMC5883_U.h>

/*
 *  Simple HTTP get webclient test
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#define USE_SERIAL Serial

#include "FastLED.h"

// WiFi Stuff
const char* ssid     = "<WIFI_SSID>";
const char* password = "<WIFI_PASSWORD>";

// LED Controller API
const char* host_address = "192.168.1.238";
const char* host_port = "3333"

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
Timer timerWiFi = { 0, 2000 };
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
  Serial.println(host);


  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  // const int httpPort = 3333;
  if (!client.connect(host_address, host_port)) {
    Serial.println("connection failed");
    //return;
  }

  // We now create a URI for the request
  String url = "/v1/devices/" + deviceName;
  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(500);

  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  //LED Stuff
  // See the Fast LED docs for your configuration.
  FastLED.addLeds<LED_TYPE, DATA_PIN, RGB>(leds, NUM_LEDS);
  LEDS.setBrightness(84);

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

//   if ( TimerExpired ( & timerLED ) )
//   {
//       if (currentMode != myMode){
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
//  String line;
  ++value;

  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 3333;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    //return;
  }

  // We now create a URI for the request
  String url = "/v1/devices/" + deviceName;
  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(500);

  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    myMode = line;
    Serial.print(line);
  }
//  return line;

  Serial.println();
  Serial.println("closing connection");
}

void taskOnBoard ( void )
{

  digitalWrite(0, LOW);
  Serial.println("LED ON");

}

void ledOff (){
//  FastLED.clear();
  for(int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to red
    leds[i] = CRGB::Black;
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    delay(10);
  }
//  leds[0] = CRGB::Black;
//  FastLED.show();
//digitalWrite(0, HIGH);
//Serial.println("LED OFF");
}

void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } }

void cylon(){
    static uint8_t hue = 0;
  Serial.print("x");
  // First slide the led in one direction
  for(int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to red
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    delay(10);
  }
  Serial.print("x");

  // Now go in the other direction.
  for(int i = (NUM_LEDS)-1; i >= 0; i--) {
    // Set the i'th led to red
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    delay(10);
  }
}

void staticColor(String hexValue){

  long int rgb=stol(hexValue,0,16);
  byte RED=(byte)(rgb>>16);
  byte GREEN=(byte)(rgb>>8);
  byte BLUE=(byte)(rgb);

  for(int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to red
    leds[i] = CHSV(RED, GREEN, BLUE);
    // Show the leds
    FastLED.show();
  }

}
