
// LPF is 1uF ceramic between pin 10 and Ground
// pin 10 is PORT 1 pin 12 on Minima
#include "LoveButton.h"


void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("\n\n\n*** Starting R4_CTSU_Test.ino***\n\n\n");
  pinMode(13, OUTPUT);
  love.begin();
}

void loop() {
  static bool oldTouch = false;
  static uint8_t ledState = LOW;
  bool touch = love.read();
  if (touch && !oldTouch) {
    ledState = 1 - ledState;
    digitalWrite(13, ledState);
    Serial.print("Touch : ");
    Serial.println(millis());
  }
  oldTouch = touch;
  delay(50);  // for debounce a little
}



