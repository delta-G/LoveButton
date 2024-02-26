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

This program will just print out the counts from the reference
and sensor clocks and the difference between them
The default setting for the touch threshold (based on the diff reading)
is 23000.  If you find that you get different readings then use the 
love.setThreshold(t); method to set a new threshold. 
*/
#include "LoveButton.h"

unsigned long debugDelay = 500;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("\n\n\n*** Starting LoveButton/examples/Debug.ino***\n\n\n");
  love.begin();
}

void loop() {
  static unsigned long lastTime = millis();
  if (millis() - lastTime >= debugDelay) {
    Serial.println(love.debug());
    lastTime = millis();
  }
}
