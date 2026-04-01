// ============================================================
//  PIR_Player
//  PIR-gesteuerter Audio-Player ohne Sleep
//  ATtiny412 + DFPlayer Mini + Transistor
//
//  Bei Bewegung: DFPlayer einschalten, zufälligen Track spielen,
//  nach INTERVAL ausschalten.
//
//  Pin-Belegung (ATtiny412):
//  PA1 (Pin 4) = PIR OUT
//  PA3 (Pin 7) = Transistor Gate (DFPlayer Power)
//  PA6 (Pin 2) = UART TX → DFPlayer RX (1kΩ in Serie!)
//  PA7 (Pin 3) = UART RX ← DFPlayer TX
// ============================================================

#include "JTS_DFPlayerTiny.h"

#define PIN_POWER  PIN_PA3
#define PIN_PIR    PIN_PA1

const uint16_t INTERVAL = 5000;  // ms nach Track bis Player aus
const uint8_t  VOLUME   = 25;

JTS_DFPlayerTiny dfp;

bool lastPirState        = false;
bool pirActive           = true;
uint16_t totalTracks     = 0;
unsigned long prevDetect = 0;

void powerOn()  { digitalWrite(PIN_POWER, HIGH); }
void powerOff() { digitalWrite(PIN_POWER, LOW);  }

void setup() {
    pinMode(PIN_PIR, INPUT);
    pinMode(PIN_POWER, OUTPUT);

    powerOn();
    dfp.begin(Serial, 9600);
    delay(100);
    dfp.waitForInit();
    dfp.volume(VOLUME);
    totalTracks = dfp.trackCount();
    dfp.stop();
    powerOff();

    randomSeed(micros());
}

void loop() {
    unsigned long now = millis();
    bool pirState = digitalRead(PIN_PIR);

    if (pirActive && !lastPirState && pirState) {
        pirActive = false;
        powerOn();
        delay(100);
        dfp.waitForInit();
        dfp.volume(VOLUME);
        dfp.playRandom(totalTracks);
        prevDetect = now;
    }

    if (!pirActive && (now - prevDetect) > INTERVAL) {
        dfp.stop();
        delay(200);
        powerOff();
        pirActive = true;
    }

    lastPirState = pirState;
    delay(50);
}
