I made several changes from Jeboo's version
* Replaced FastLED with the Adafruit NeoPixel libraries
* Replaced the EEPROM storage with Preferences for the set-able parking distance
* All new logic for how many LEDs to light, in which color, based on the LED_COUNT, so it looks reasonable with as few as 10 or as many as 33
