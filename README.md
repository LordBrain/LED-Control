# LED-Control

A simple web service that allows you to control LED light strips using a rest API. Using a Ardiuno with wifi to control the LED strip.

### Rest API

The rest endpoints are:
* / - The web page used for controlling the strips. It will have a drop down list of all the devices and a color selection wheel.
* /images - Where the color selection wheel is (other images can go there as well, if you want).
* /ping - Alive health check. Will return 'pong'.
* /v1/devices/ - Main rest endpoint.
  * POST - Adds a device to the service. Returns a 201 if added correctly, 200 if its already added.
  * /:deviceID
    * GET - Returns the current mode for the device or Hex color code to display. Returns a 200 if okay, 500 for internal server error.
    * PUT - Updates the mode for the device. Returns a 200 if okay, 500 for internal server error.

### Ardiuno - To Do

The idea is the a Ardiuno will connect to the Rest API, register itself. If its already registered, it will see it and start checking for what it should be doing.

The [Adafruit Feather with WiFi](https://www.adafruit.com/product/2821) seems like a good fit for this.
