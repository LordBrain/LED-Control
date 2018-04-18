# LED-Control

A simple web service that allows you to control LED light strips using a rest API. Using a Ardiuno with wifi to control the LED strip.

It writes the devices and mode to a local file for persistence. No need for a database.

## Docker Container

This repo builds a container on docker hub. You can use that if you do not want to build this go project your self. [Get the container here](https://hub.docker.com/r/blee1170/led-control/).

## Rest API

By default it runs on port 3333. Set a environment variable for PORT to the port number you want to run on.

The rest endpoints are:
* / - The web page used for controlling the strips. It will have a drop down list of all the devices and a color selection wheel.
* /images - Where the color selection wheel is (other images can go there as well, if you want).
* /ping - Alive health check. Will return 'pong'.
* /v1/devices/:deviceID - Main rest endpoint.
  * POST - Adds a device to the service. Returns a 201 if added correctly, 200 if its already added.
  * GET - Returns the current mode for the device or Hex color code to display. Returns a 200 if okay, 500 for internal server error.
  * PUT - Updates the mode for the device. Returns a 200 if okay, 500 for internal server error.

## Ardiuno

The idea is the a Ardiuno will connect to the Rest API, register itself. If its already registered, it will see it and start checking for what colors it should set the LEDs to.

#### Adafruit Feather
This [Adafruit Feather](https://www.adafruit.com/product/2821) has wifi built into it. It makes it simple to connect it to the API server and it has enough power to run a 5 meter strip of 60 LEDs (WS2812B). I followed the wiring of the LEDs from [Adafruit Uber guide](https://learn.adafruit.com/adafruit-neopixel-uberguide) it get it working.
