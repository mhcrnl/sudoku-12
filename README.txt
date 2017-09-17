
Paul Solleza
Peter Cvijovic
LBL EA1

-------------------------------------------------------------------------------------------
Accessories:
* Arduino Mega Board (AMG) x1 w/ breadboard
* TFT LCD screen x1
* Sparkfun Thumb Joystick x1

-------------------------------------------------------------------------------------------
Wiring instructions:
AMG GND <--> BB GND bus
AMG +5V <--> BB +5V bus

TFT LCD screen GND <------> BB GND bus
TFT LCD screen VCC <------> BB +5V bus
TFT LCD screen RESET <----> AMG Pin 8
TFT LCD screen D/C <------> AMG Pin 7
TFT LCD screen CARD_CS <--> AMG Pin 5
TFT LCD screen TFT_CS <---> AMG Pin 6
TFT LCD screen MOSI <-----> AMG Pin 51
TFT LCD screen SCK <------> AMG Pin 52
TFT LCD screen MISO <-----> AMG Pin 50
TFT LCD screen LITE <-----> BB +5V bus

Sparkfun Thumb Joystick VCC <---> BB +5V bus
Sparkfun Thumb Joystick VERT <--> AMG Analog Pin A0
Sparkfun Thumb Joystick HOR <---> AMG Analog Pin A1
Sparkfun Thumb Joystick SEL <---> AMG Digital Pin 9
Sparkfun Thumb Joystick GND <---> BB GND bus

AMG Analog Pin A7 <---> n/a

-------------------------------------------------------------------------------------------
Running instruction:
* open terminal

* change directory to the folder's location
note: its easier to copy the folder and paste it to home and then type "cd ~/<folder_name>"

* type in "make upload" to compile and program file unto Arduino

* type in "serial-mon" if you wish to see the solution for verification

* type in "Ctrl+'A'" then 'X' to exit serial-mon

* select difficulty. use the joystick to navigate and press on it to select

* press on the joystick to change number on grid. value changes from 0(NULL) to 9

-------------------------------------------------------------------------------------------
Assumptions in implementation:
* user doesn't touch the joystick while calibrating

-------------------------------------------------------------------------------------------
Problems encountered:
* on the board screen, the cursor can appear on the bottom right empty cell for a split sec
  --> does nothing

-------------------------------------------------------------------------------------------
Additional functionality:
* fixed numbers are colored RED

* user can try again if their solution was incorrect

-------------------------------------------------------------------------------------------
Acknowledgements:
* uses the makefile provided in class

* mahiya http://www.cplusplus.com/forum/beginner/76616/

* https://en.wikipedia.org/wiki/Sudoku_solving_algorithms

-------------------------------------------------------------------------------------------
Notes:
* generate_grid() can use a selection of other pre existing sudokus for greater randomness

* the way difficulty is chosen could be improved.

* nothing special about the wiring except analog pin 7 must not be connected to anything

* test_unique() returns false if solution remains true after removing the number
