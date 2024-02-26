/*

LoveButton  --  Capacitive Touch Sensing for the Love Pin on the Arduino UNO-R4 Minima
     Copyright (C) 2023  David C.

     R4-Wifi compatability added by Winnie S.

     This program is free software: you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program.  If not, see <http://www.gnu.org/licenses/>.

     */

/*
This example requires installing a 1uF ceramic capacitor between 
pin 10 and ground on the Arduino UNO-R4 Minima or between 
pin 7 and ground on the Arduino UNO-R4 WiFi.  

Try to ensure that the leads are kept as short as possible.  

Touch the small heart shaped blob of solder on the back of the board
to toggle the built-in LED on pin 13.  
*/

#include "LoveButton.h"

bool oldTouch = false;
uint8_t ledState = LOW;

void setup() {
  pinMode(13, OUTPUT);
  love.begin();  // Start the capacitive sensor
}

void loop() {
  // read the touch sensor and store the result
  bool touch = love.read();
  if (touch && !oldTouch) {
    // if there's a new touch.
    // toggle the led state
    ledState = 1 - ledState;
    digitalWrite(13, ledState);
  }
  oldTouch = touch;  // save the state for next time
  delay(50);         // for debounce a little
}
