#define PRINT_REG(BUCKET, REGISTER) \
  do { \
    uint32_t t = BUCKET->REGISTER; \
    Serial.print(#REGISTER " : 0x"); \
    Serial.println(t, HEX); \
  } while (false)

#include "EventLinkInterrupt.h"
// LPF is 1uF ceramic between pin 10 and Ground
// pin 10 is PORT 1 pin 12 on Minima

volatile boolean newData = false;
volatile uint16_t sCounter = 0;
volatile uint16_t rCounter = 0;

int ctsurdEventLinkIndex = 0;
int ctsuwrEventLinkIndex = 0;

int ctsufnEventLinkIndex = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("\n\n\n*** Starting R4_CTSU_Test.ino***\n\n\n");
  pinMode(13, OUTPUT);
  initialCTSUsetup();
  startCTSUmeasure();
}

void loop() {
  handleSerial();
  // if (newData) {
  //   newData = false;
  //   noInterrupts();
  //   uint32_t s = sCounter;
  //   uint32_t r = rCounter;
  //   interrupts();
  //   Serial.print("Result : ");
  //   Serial.print(s);
  //   Serial.print(" : ");
  //   Serial.print(r);
  //   Serial.print(" : ");
  //   Serial.println();
  //   // startCTSUmeasure();
  // }
}

void CTSUWR_handler() {
  resetEventLink(ctsuwrEventLinkIndex);
  R_CTSU->CTSUMCH0 = 0;
  R_CTSU->CTSUSO1 = 0x0F00;
}

void CTSURD_handler() {
  resetEventLink(ctsurdEventLinkIndex);
  sCounter = R_CTSU->CTSUSC;
  rCounter = R_CTSU->CTSURC;
  static bool oldTouch = false;
  bool touch = (sCounter > 22000);
  if(touch && !oldTouch){
    digitalWrite(13, !digitalRead(13));
  }
  oldTouch = touch;
  startCTSUmeasure();
}

void startCTSUmeasure() {
  R_CTSU->CTSUMCH0 = 0;
  R_CTSU->CTSUCR0 = 1;
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
  R_CTSU->CTSUSDPRS = 0x63;
  R_CTSU->CTSUSST = 0x10;
  R_CTSU->CTSUCHAC[0] = 1;
  R_CTSU->CTSUDCLKC = 0x30;

  R_CTSU->CTSUMCH0 = 0;

  // CTSUWR is event 0x42
  // CTSURD is event 0x43
  // CTSUFN is event 0x44
  ctsurdEventLinkIndex = attachEventLinkInterrupt(0x43, CTSURD_handler);
  ctsuwrEventLinkIndex = attachEventLinkInterrupt(0x42, CTSUWR_handler);
}

void handlePacket(char *buf) {
  uint8_t argStart = 0;
  // find the colon
  for (int i = 0; i < strlen(buf); i++) {
    if (buf[i] == ':') {
      argStart = i + 1;
      break;
    }
  }
  int reg = atoi(buf + 1);
  uint32_t arg = strtoul(buf + argStart, NULL, HEX);
  if (reg > 0) {
    Serial.print("Setting : reg ");
    Serial.print(reg);
    Serial.print("  :  0x");
    Serial.println(arg, HEX);
  }
  switch (reg) {
    case 0:
      // reinterpret arg as decimal
      arg = atoi(buf + argStart);
      Serial.print("Calling Command : ");
      Serial.println(arg);
      // regular commands
      switch (arg) {
        case 0:
          // Print all registers
          for (int i = 1; i <= 24; i++) {
            printRegister(i);
          }
          break;
        case 1 ... 24:
          printRegister(arg);
          break;
        case 101:
          // set TSCAP pin as OUTPUT - LOW:
          R_PFS->PORT[1].PIN[12].PmnPFS = (1 << R_PFS_PORT_PIN_PmnPFS_PDR_Pos);
          break;
        case 102:
          // set TSCAP pin to TSCAP function
          R_PFS->PORT[1].PIN[12].PmnPFS = (1 << R_PFS_PORT_PIN_PmnPFS_PMR_Pos) | (12 << R_PFS_PORT_PIN_PmnPFS_PSEL_Pos);
          break;
        case 103:
          //enable CTSU input clock in MSTPCRC
          R_MSTP->MSTPCRC &= ~(1 << R_MSTP_MSTPCRC_MSTPC3_Pos);
          break;
        case 104:
          // attach the CTSURD handler
          // CTSURD is event 0x43
          // CTSUWR is event 0x42
          // CTSUFN is event 0x44
          ctsurdEventLinkIndex = attachEventLinkInterrupt(0x43, CTSURD_handler);
          break;
        default:
          break;
      }
      break;
    case 1:
      R_CTSU->CTSUCR0 = arg & 0xFF;
      break;
    case 2:
      R_CTSU->CTSUCR1 = arg & 0xFF;
      break;
    case 3:
      R_CTSU->CTSUSDPRS = arg & 0xFF;
      break;
    case 4:
      R_CTSU->CTSUSST = arg & 0xFF;
      break;
    case 5:
      R_CTSU->CTSUMCH0 = arg & 0xFF;
      break;
    case 6:
      R_CTSU->CTSUMCH1 = arg & 0xFF;
      break;
    case 7:
      R_CTSU->CTSUCHAC[0] = arg & 0xFF;
      break;
    case 8:
      R_CTSU->CTSUCHAC[1] = arg & 0xFF;
      break;
    case 9:
      R_CTSU->CTSUCHAC[2] = arg & 0xFF;
      break;
    case 10:
      R_CTSU->CTSUCHAC[3] = arg & 0xFF;
      break;
    case 11:
      R_CTSU->CTSUCHAC[4] = arg & 0xFF;
      break;
    case 12:
      R_CTSU->CTSUCHTRC[0] = arg & 0xFF;
      break;
    case 13:
      R_CTSU->CTSUCHTRC[1] = arg & 0xFF;
      break;
    case 14:
      R_CTSU->CTSUCHTRC[2] = arg & 0xFF;
      break;
    case 15:
      R_CTSU->CTSUCHTRC[3] = arg & 0xFF;
      break;
    case 16:
      R_CTSU->CTSUCHTRC[4] = arg & 0xFF;
      break;
    case 17:
      R_CTSU->CTSUDCLKC = arg & 0xFF;
      break;
    case 18:
      R_CTSU->CTSUST = arg & 0xFF;
      break;
    case 19:
      R_CTSU->CTSUSSC = arg & 0xFFFF;
      break;
    case 20:
      R_CTSU->CTSUSO0 = arg & 0xFFFF;
      break;
    case 21:
      R_CTSU->CTSUSO1 = arg & 0xFFFF;
      break;
    default:
      break;
  }
}

void handleSerial() {
  static char buffer[32];
  static uint8_t index = 0;
  static bool receiving = false;

  if (Serial.available()) {
    char c = Serial.read();

    if (c == '<') {
      // start of message
      receiving = true;
      index = 0;
      buffer[0] = 0;
    }
    if (receiving) {
      buffer[index] = c;
      buffer[++index] = 0;
      if (c == '>') {
        // end of packet
        receiving = false;
        handlePacket(buffer);
      }
    }
  }
}

void printRegister(int reg) {
  Serial.print(reg);
  Serial.print(" - ");
  switch (reg) {
    case 1:
      PRINT_REG(R_CTSU, CTSUCR0);
      break;
    case 2:
      PRINT_REG(R_CTSU, CTSUCR1);
      break;
    case 3:
      PRINT_REG(R_CTSU, CTSUSDPRS);
      break;
    case 4:
      PRINT_REG(R_CTSU, CTSUSST);
      break;
    case 5:
      PRINT_REG(R_CTSU, CTSUMCH0);
      break;
    case 6:
      PRINT_REG(R_CTSU, CTSUMCH1);
      break;
    case 7:
      PRINT_REG(R_CTSU, CTSUCHAC[0]);
      break;
    case 8:
      PRINT_REG(R_CTSU, CTSUCHAC[1]);
      break;
    case 9:
      PRINT_REG(R_CTSU, CTSUCHAC[2]);
      break;
    case 10:
      PRINT_REG(R_CTSU, CTSUCHAC[3]);
      break;
    case 11:
      PRINT_REG(R_CTSU, CTSUCHAC[4]);
      break;
    case 12:
      PRINT_REG(R_CTSU, CTSUCHTRC[0]);
      break;
    case 13:
      PRINT_REG(R_CTSU, CTSUCHTRC[1]);
      break;
    case 14:
      PRINT_REG(R_CTSU, CTSUCHTRC[2]);
      break;
    case 15:
      PRINT_REG(R_CTSU, CTSUCHTRC[3]);
      break;
    case 16:
      PRINT_REG(R_CTSU, CTSUCHTRC[4]);
      break;
    case 17:
      PRINT_REG(R_CTSU, CTSUDCLKC);
      break;
    case 18:
      PRINT_REG(R_CTSU, CTSUST);
      break;
    case 19:
      PRINT_REG(R_CTSU, CTSUSSC);
      break;
    case 20:
      PRINT_REG(R_CTSU, CTSUSO0);
      break;
    case 21:
      PRINT_REG(R_CTSU, CTSUSO1);
      break;
    case 22:
      PRINT_REG(R_CTSU, CTSUSC);
      break;
    case 23:
      PRINT_REG(R_CTSU, CTSURC);
      break;
    case 24:
      PRINT_REG(R_CTSU, CTSUERRS);
      break;
    default:
      break;
  }
}