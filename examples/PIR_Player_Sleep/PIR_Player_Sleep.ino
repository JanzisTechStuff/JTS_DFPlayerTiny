// ============================================================
//  PIR_Player_Sleep
//  PIR-gesteuerter Audio-Player mit maximalem Stromsparmodus
//  ATtiny412 + DFPlayer Mini + Transistor
//
//  ATtiny schläft im POWER_DOWN-Modus (~0.1µA).
//  PIR-Interrupt weckt ihn auf, Track wird gespielt,
//  danach wieder schlafen.
//
//  Pin-Belegung (ATtiny412):
//  PA1 (Pin 4) = PIR OUT
//  PA3 (Pin 7) = Transistor Gate (DFPlayer Power)
//  PA6 (Pin 2) = UART TX → DFPlayer RX (1kΩ in Serie!)
//  PA7 (Pin 3) = UART RX ← DFPlayer TX
// ============================================================

#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "JTS_DFPlayerTiny.h"

#define PIN_POWER  PIN_PA3
#define PIN_PIR    PIN_PA1

const uint16_t INTERVAL = 5000;  // ms nach Track bis Player aus
const uint8_t  VOLUME   = 25;

JTS_DFPlayerTiny dfp;

volatile bool pirTriggered = false;
uint16_t totalTracks       = 0;

// ── PIR Interrupt ─────────────────────────────────────────────
ISR(PORTA_PORT_vect) {
    if (PORTA.INTFLAGS & PIN1_bm) {
        PORTA.INTFLAGS = PIN1_bm;
        if (PORTA.IN & PIN1_bm) {
            pirTriggered = true;
        }
    }
}

// ── Sleep ─────────────────────────────────────────────────────
void goToSleep() {
    while (PORTA.IN & PIN1_bm) delay(50);  // warten bis PIR LOW
    delay(100);
    pirTriggered = false;
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sei();
    sleep_cpu();
    sleep_disable();
}

void powerOn()  { digitalWrite(PIN_POWER, HIGH); }
void powerOff() { digitalWrite(PIN_POWER, LOW);  }

void setup() {
    pinMode(PIN_PIR, INPUT);
    pinMode(PIN_POWER, OUTPUT);

    // PIR Interrupt: beide Flanken, ISR filtert auf steigende
    PORTA.PIN1CTRL = PORT_ISC_BOTHEDGES_gc;

    powerOn();
    dfp.begin(Serial, 9600);
    delay(100);
    dfp.waitForInit();
    dfp.volume(VOLUME);
    totalTracks = dfp.trackCount();
    dfp.stop();
    powerOff();

    goToSleep();
}

void loop() {
    if (pirTriggered) {
        pirTriggered = false;

        powerOn();
        delay(100);
        dfp.waitForInit();
        dfp.volume(VOLUME);
        dfp.playRandom(totalTracks);

        delay(INTERVAL);

        dfp.stop();
        delay(200);
        powerOff();

        goToSleep();
    }
}
