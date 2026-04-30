# ESP RASPBERRY PI SMART-HOME PROJECT FOR ERASMUS

This is a 2-week project that 6 people worked on. 
We had a model of a house already at disposal that other people before us worked on. The cable management wasn't the greatest so one of our guys had completely redone the cables and connections. Looks alot better.

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
- 4x4 keyboard 

Now that we had the components connected, we started working on the Raspberry Pi by installing a 32-bit
version of Raspberry Pi Lite OS because we don't need an UI.

After that we downloaded and installed OpenHABian with manual installation and also JAVA 17.
We had to work with an older model because the Pi was a model 3.
Now we can work on the ESPs.

We needed a way for the ESPs to communicate with the Pi so it can send and recieve information.
For that we used a publish-subscribe based messaging protocol called MQTT, which was installed on the Pi, and on the ESPs using a library. Then we connected both of the devices to a hotspot connection.
> because the wifi here wasn't great at all, we had to use our hotspot.

And now we could send messages and commands to the ESP. 🥳\

*Now the fun part, writing the code.*
> DISCLAMER, THIS IS 100% HUMAN CODE

## The process of programming...

First thing first we needed to connect the ESP to the internet. the ESP had a built-in wifi module which was great. We only needed to use the local library and then define the SSID and password.

Then we connected the ESP to the MQTT and the comms were operational.

We programmed the LEDs so we could turn them on and off using the OpenHab platform portal. That was inside and outside light.
> OpenHab platfrom portal from now on will just be the "site" or "dashboard"

After that we made the humidity sensor functional and made it show the % of the humidity in the house show on the site.

Now we had to implement a function to open the door. We did that with a servo motor. We can open the door and
close the door and also lock it on the site. When the **LOCK** function is on, nothing is able to open the door.

Because it is boresome to manually open the door, we used an Ultrasound sensor that was hanging on the ceiling in front of the door, so it can detect movement and automaticly open the door
> NOTE: the Ultrasound sensor is kinda broken, blame the group before us.

It is great we can lock the door, but what if someone tries to break in? For that we have a laser and a photoresistor. The laser aimed straight at the photoresistor right across the hallway, so when someone passes through, the photoresistor detects that the light beam is broken and sends a signal to the ESP, which sends it straight to the site. The laser is on **ONLY** when the LOCK function is active.

Now we implement a buzzer for the alarm. If the LOCK is on, and someone passes through the laser, the buzzer starts buzzin for a few seconds and on the OpenHab dashboard it shows an intruder alert.

Then we started using the keypad. The idea was that the LOCK is always on and the only way to turn the LOCK off is either through the dashboard site OR the keypad connected to the Raspberry Pi. We wanted to connect it to the ESP, but we needed 8 usable pins. ESP had only 7. 
So once we type the right number combination on the keypad, the door will be unlocked and opened and the laser will be turned off.





