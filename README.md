# VersionOne Status Light
This is an Arduino sketch for the VersionOne Status Light that shows recent build and server statuses.

![V1StatusLight](../Doc/V1StatusLight.jpg)

It was originally written for the [Adafruit Feather M0 WiFi - ATSAMD21 + ATWINC1500](https://www.adafruit.com/product/3010), and more recently updated for the [Adafruit HUZZAH32 – ESP32 Feather Board](https://www.adafruit.com/product/3405) since the M0 won't connect to the new WiFi installed in June of 2017.
## Prerequisites
* [Arduino IDE](https://www.arduino.cc/en/Main/Software) 1.8.2 or higher
* ArduinoJson library (installed within the IDE)
* Drivers and Arduino boards as appropriate.  See the Adafruit site for specific directions.
## Software
The sketch polls the [V1ServerStatus API](https://github.com/versionone/V1ServerStatus) to retrieve the Continuum and Zabbix statues for the 12 most recent events for each.  The status is then displayed on the Neopixel ring.  The blue LED just pulses to indicate that the main loop is running to catch any crashes.
## Hardware
* [Adafruit HUZZAH32 – ESP32 Feather Board](https://www.adafruit.com/product/3405)
* [NeoPixel Ring](https://www.adafruit.com/product/1586)
* Blue LED, with 220 Ohm current-limiting resistor (for 3.3V from HUZZAH)
* 1000uF Capacitor and 330 Ohn resistor for Neopixel (see the [guide](https://learn.adafruit.com/adafruit-neopixel-uberguide/best-practices))
* Small Breadboard
* 3D printed bracket.  [SketchUp File](../Doc/24NeoPixel.skp), [GCode file](../Doc/24NeoPixel.gcode)

![NeoPixel24Model](../Doc/24Neopixel.PNG)
