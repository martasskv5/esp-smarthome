# ESP SMART HOME PROJECT FOR ERAZMUS

This is a 2-week project that 6 people worked on. We had a model of a house already at disposal that
others before us worked on. The cable management wasn't greatest so one of our guys had completely redone
the cables.

After that we connected 2 ESP8266 to a breadboard and began the assembly of the components.

### Components list:
- Solar Panel
- Servo Motor
- Humidity Sensor
- 2 LEDs
- 2 Batteries
- Laser
- Buzzer
- Photoresistor
- Ultrasound Sensor

Now that we had the components connected, we started working on the Raspberry Pi by installing a 32-bit
version of Raspberry Pi Lite OS because we don't need an UI.

After that we downloaded and installed OpenHABian with manual installation. Now we can work on the ESPs.

We needed a way for the ESPs to communicate with the Pi so it can send and recieve information.
For that we used a publish-subscribe based messaging protocol called MQTT, which was installed on the Pi,
and also connected both of the devices to a hotspot connection
> because the wifi here wasn't the greatest.

And now we could send messages and commands to the ESP. 🥳\

*Now the fun part, writing the code.*
> DISCLAMER, THIS IS 100% HUMAN CODE

## The process of programming...

First we made the LEDs functional. We could turn them on and off using the OpenHab platform portal.
After that we made the humidity sensor functional and made it show the % of the humidity on the OpenHab site.


