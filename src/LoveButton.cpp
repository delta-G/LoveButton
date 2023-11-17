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

#include "LoveButton.h"

LoveButton love;

namespace LB_NAMESPACE {

int ctsurdEventLinkIndex = 0;
int ctsuwrEventLinkIndex = 0;

volatile uint16_t sCounter;
volatile uint16_t rCounter;

volatile boolean touch;

void CTSUWR_handler() {
  // we need this interrupt to trigger the CTSU to go to state 3.
  resetEventLink(ctsuwrEventLinkIndex);
  R_CTSU->CTSUMCH0 = 0;
  R_CTSU->CTSUSO1 = 0x0F00;
}

void CTSURD_handler() {
  resetEventLink(ctsurdEventLinkIndex);
  sCounter = R_CTSU->CTSUSC;
  // Must read CTSURC even if we don't use it in order for the unit to move on
  rCounter = R_CTSU->CTSURC;
  touch = (sCounter - rCounter > love.threshold);
  startCTSUmeasure();
}

void startCTSUmeasure() {
  R_CTSU->CTSUMCH0 = 0;  // select pin TS00
  R_CTSU->CTSUCR0 = 3;   // software start measurement wait for trigger
}

}

char* LoveButton::debug() {
  static char rv[40];
  noInterrupts();
  sprintf(rv, "rc= %d : sc= %d : diff= %d", LB_NAMESPACE::rCounter, LB_NAMESPACE::sCounter, (LB_NAMESPACE::sCounter - LB_NAMESPACE::rCounter));
  interrupts();
  return rv;
}

bool LoveButton::read() {
  return LB_NAMESPACE::touch;
}

void LoveButton::setThreshold(uint16_t t){
  threshold = t;
}

void LoveButton::begin() {
  static bool hasBegun = false;
  if(!hasBegun){
    hasBegun = true;
     // Follow the flow chart Fig 41.9
     // Step 1: Discharge LPF (set TSCAP as OUTPUT LOW.)
     R_PFS->PORT[1].PIN[12].PmnPFS = (1 << R_PFS_PORT_PIN_PmnPFS_PDR_Pos);
     delay(100);

     // Step 2: Setup I/O port PmnPFR registers
     // Love pin is 204 == TS00
     // set 204 pin to TS00 function
     R_PFS->PORT[2].PIN[4].PmnPFS = (1 << R_PFS_PORT_PIN_PmnPFS_PMR_Pos) | (12 << R_PFS_PORT_PIN_PmnPFS_PSEL_Pos);
     // set TSCAP pin to TSCAP function
     R_PFS->PORT[1].PIN[12].PmnPFS = (1 << R_PFS_PORT_PIN_PmnPFS_PMR_Pos) | (12 << R_PFS_PORT_PIN_PmnPFS_PSEL_Pos);

     // Step 3: Enable CTSU in MSTPCRC bit MSTPC3 to 0
     R_MSTP->MSTPCRC &= ~(1 << R_MSTP_MSTPCRC_MSTPC3_Pos);

     // Step 4: Set CTSU Power Supply (CTSUCR1 register)
     R_CTSU->CTSUCR1 = 0;  // all 0's work for now

     // Step 5: Set CTSU Base Clock (CTSUCR1 and CTSUSO1 registers)
     R_CTSU->CTSUSO1 = 0x0F00;

     // Step 6: Power On CTSU (set bits CTSUPON and CTSUCSW in CTSUCR1 at the same time)
     R_CTSU->CTSUCR1 = 3;

     // Step 7: Wait for stabilization (Whatever that means...)
     delay(100);

     // setup other registers:
     R_CTSU->CTSUSDPRS = 0x63;  //recommended settings with noise reduction off
     R_CTSU->CTSUSST = 0x10;    // data sheet says set value to this only
     R_CTSU->CTSUCHAC[0] = 1;   // enable pin TS00 for measurement
     R_CTSU->CTSUDCLKC = 0x30;  // data sheet dictates these settings.

     R_CTSU->CTSUMCH0 = 0;  // select pin TS00

     // CTSUWR is event 0x42
     // CTSURD is event 0x43
     // CTSUFN is event 0x44
     LB_NAMESPACE::ctsurdEventLinkIndex = attachEventLinkInterrupt(0x43, LB_NAMESPACE::CTSURD_handler);
     LB_NAMESPACE::ctsuwrEventLinkIndex = attachEventLinkInterrupt(0x42, LB_NAMESPACE::CTSUWR_handler);
     // Enable Event Link Controller in Master Stop Register
     R_MSTP->MSTPCRC &= ~(1 << R_MSTP_MSTPCRC_MSTPC14_Pos);
     // The ELC register for CTSU is ELSR18
     // The event link signal for AGT0 underflow is 0x1E
     R_ELC->ELSR[18].HA = 0x1E;
     // enable ELC
     R_ELC->ELCR = (1 << R_ELC_ELCR_ELCON_Pos);

     LB_NAMESPACE::startCTSUmeasure();
  }
}
