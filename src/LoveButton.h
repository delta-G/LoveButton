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

#ifndef LOVEBUTTON_H
#define LOVEBUTTON_H

#if !defined(ARDUINO_UNOR4_WIFI) && !defined(ARDUINO_UNOR4_MINIMA)
#error Sorry, but LoveButton only works on the Arduino UNO-R4 Minima and Arduino UNO-R4 WiFi
#endif

#include "Arduino.h"
#include "EventLinkInterrupt.h"
// LPF is 1uF ceramic between pin 10 and Ground on Minima or between pin 7 and Ground on Wifi. This is PORT 1 pin 12

class LoveButton {
public:
  uint16_t threshold;
  LoveButton() : threshold(23000) {}

  void begin();
  bool read();
  char* debug();
  void setThreshold(uint16_t t);
};

extern LoveButton love;
namespace LB_NAMESPACE {

void startCTSUmeasure();
void CTSURD_handler();
void CTSUWR_handler();
}

#endif  //LOVEBUTTON_H
