#include "EventLinkInterrupt.h"
// LPF is 1uF ceramic between pin 10 and Ground
// pin 10 is PORT 1 pin 12 on Minima

int ctsurdEventLinkIndex = 0;
int ctsuwrEventLinkIndex = 0;

volatile bool touch = false;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("\n\n\n*** Starting R4_CTSU_Test.ino***\n\n\n");
  pinMode(13, OUTPUT);
  initialCTSUsetup();
  initAGT1();
  startCTSUmeasure();
}

void loop() {
  static bool oldTouch = false;
  static uint8_t ledState = LOW;
  if (touch && !oldTouch) {
    ledState = 1 - ledState;
    digitalWrite(13, ledState);
  }
  oldTouch = touch;
  delay(50);  // for debounce a little
}

void CTSUWR_handler() {
  // we need this interrupt to trigger the CTSU to go to state 3.
  resetEventLink(ctsuwrEventLinkIndex);
  R_CTSU->CTSUMCH0 = 0;
  R_CTSU->CTSUSO1 = 0x0F00;
}

void CTSURD_handler() {
  resetEventLink(ctsurdEventLinkIndex);
  uint16_t sCounter = R_CTSU->CTSUSC;
  // Must read CTSURC even if we don't use it in order for the unit to move on
  uint16_t rCounter = R_CTSU->CTSURC;
  touch = (sCounter > 22000);
  startCTSUmeasure();
}

void startCTSUmeasure() {
  R_CTSU->CTSUMCH0 = 0;  // select pin TS00
  R_CTSU->CTSUCR0 = 3;   // software start measurement wait for trigger
}

void initialCTSUsetup() {
  // Follow the flow chart Fig 41.9
  // Step 1: Discharge LPF (set TSCAP as OUTPUT LOW.)
  R_PFS->PORT[1].PIN[12].PmnPFS = (1 << R_PFS_PORT_PIN_PmnPFS_PDR_Pos);
  delay(1000);

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
  delay(1000);

  // setup other registers:
  R_CTSU->CTSUSDPRS = 0x63;  //recommended settings with noise reduction off
  R_CTSU->CTSUSST = 0x10;    // data sheet says set value to this only
  R_CTSU->CTSUCHAC[0] = 1;   // enable pin TS00 for measurement
  R_CTSU->CTSUDCLKC = 0x30;  // data sheet dictates these settings.

  R_CTSU->CTSUMCH0 = 0;  // select pin TS00

  // CTSUWR is event 0x42
  // CTSURD is event 0x43
  // CTSUFN is event 0x44
  ctsurdEventLinkIndex = attachEventLinkInterrupt(0x43, CTSURD_handler);
  ctsuwrEventLinkIndex = attachEventLinkInterrupt(0x42, CTSUWR_handler);
}

void initAGT1() {
  // enable the timer in Module Stop Control Register D
  R_MSTP->MSTPCRD &= ~(1 << R_MSTP_MSTPCRD_MSTPD2_Pos);
  //  Make sure timer is stopped while we adjust registers.
  R_AGT1->AGTCR = 0;

  // We're using R_AGT1, but all the positions and bitmasks are defined as R_AGT0
  // set mode register 1
  //(-) (TCK[2:0]) (TEDGPL) (TMOD[2:0])
  //  Use TIMER mode with the LOCO clock (best we can do since Arduino doesn't have crystal for SOSC)
  R_AGT1->AGTMR1 = (4 << R_AGT0_AGTMR1_TCK_Pos) | (0 << R_AGT0_AGTMR1_TMOD_Pos);
  // mode register 2
  // (LPM) (----) (CKS[2:0])
  R_AGT1->AGTMR2 = 0;
  // AGT I/O Control Register
  // (TIOGT[1:0]) (TIPF[1:0]) (-) (TOE) (-) (TEDGSEL)
  R_AGT1->AGTIOC = 0;
  // Event Pin Select Register
  // (-----) (EEPS) (--)
  R_AGT1->AGTISR = 0;
  // AGT Compare Match Function Select Register
  // (-) (TOPOLB) (TOEB) (TCMEB) (-) (TOPOLA) (TOEA) (TCMEA)
  R_AGT1->AGTCMSR = 0;
  // AGT Pin Select Register
  // (---) (TIES) (--) (SEL[1:0])
  R_AGT1->AGTIOSEL = 0;

  // setup 50ms period
  R_AGT1->AGTMR1 = (4 << R_AGT0_AGTMR1_TCK_Pos) | (0 << R_AGT0_AGTMR1_TMOD_Pos);
  R_AGT1->AGTMR2 = 0;
  R_AGT1->AGT = 1638;

  // Enable Event Link Controller in Master Stop Register
  R_MSTP->MSTPCRC &= ~(1<<R_MSTP_MSTPCRC_MSTPC14_Pos);
  // The ELC register for CTSU is ELSR18
  // The event link signal for AGT1 underflow is 0x21
  R_ELC->ELSR[18].HA = 0x21;
  // enable ELC 
  R_ELC->ELCR = (1 << R_ELC_ELCR_ELCON_Pos);

  // start timer
  R_AGT1->AGTCR = 1;
}