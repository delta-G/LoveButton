/*

LoveButton  --  Capacitive Touch Sensing for the Love Pin on the Arduino UNO-R4 Minima
     Copyright (C) 2023  David C.

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

#ifndef LOVEBUTTON_H
#define LOVEBUTTON_H

#include "Arduino.h"
#include "EventLinkInterrupt.h"
// LPF is 1uF ceramic between pin 10 and Ground
// pin 10 is PORT 1 pin 12 on Minima

class LoveButton {
public:
  LoveButton() {}

  void begin();
  bool read();
};

extern LoveButton love;

void startCTSUmeasure();
void CTSURD_handler();
void CTSUWR_handler();

#endif //LOVEBUTTON_H