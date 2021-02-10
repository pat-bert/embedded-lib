# embedded-lib

Collection of my own libraries for interfacing with MCU-peripherals or ICs
- [x] Library for I2C-GPIO-Expander PCF8574 with open-collector pins
* Writing/reading pins (supply mask and pins will be asserted high to be read)
* Generic interface via I2C-Callback functions
- [x] LCD Library:
* Tested with 1602 and 2004 + PCF8574
* Cursor functions: Moving to position, reading current position
* Print functions: Get/Set character at cursor, print text with optional line-wrap at position (x,y)
* Display functions: Cursor, blink, scroll, 
* Generic interface via callback functions
* Can be used with I2C-GPIO-Expander PCF8574
* No callback for parallel operation via GPIO supplied yet
* 4-bit and 8-bit mode (latter is in testing phase)
* Busy Flag checking and correct start-up delays
* Custom Character RAM write
