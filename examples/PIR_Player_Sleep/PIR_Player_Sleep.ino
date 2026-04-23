// ============================================================
//  PIR_Player_Sleep
//  PIR-controlled Audio Player with maximum power-saving mode
//  ATtiny412 + DFPlayer Mini + Transistor
//
//  ATtiny sleeps in POWER_DOWN mode (~0.1µA).
//  A PIR interrupt wakes it up, a track is played,
//  then the device goes back to sleep.
//
//  Pinout (ATtiny412):
//  PA1 (Pin 4) = PIR OUT
//  PA2 (Pin 5) = (BC337) Transistor Gate (DFPlayer Power) (220Ω in series)
//  PA6 (Pin 2) = UART TX → DFPlayer RX (1kΩ in series)
//  PA7 (Pin 3) = UART RX ← DFPlayer TX
// ============================================================

#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "JTS_DFPlayerTiny.h"

#define PIN_POWER  PIN_PA1  // Pin controlling DFPlayer power via transistor
#define PIN_PIR    PIN_PA2  // Pin connected to PIR sensor output

const uint16_t INTERVAL = 5000;  // Time in ms to keep player on after track finishes
const uint8_t  VOLUME   = 20;    // Volume level for playback (0-30)

JTS_DFPlayerTiny dfp;             // DFPlayer object
volatile bool pirTriggered = false; // Flag set by PIR interrupt
uint16_t totalTracks       = 0;    // Total number of tracks on SD card

// ── PIR Interrupt Service Routine ─────────────────────────────
ISR(PORTA_PORT_vect) {
    // Check if the interrupt is from PIR pin (PA1)
    if (PORTA.INTFLAGS & PIN2_bm) {
        PORTA.INTFLAGS = PIN2_bm; // Clear interrupt flag
        pirTriggered = true;  // Set flag to trigger playback
    }
}

// ── Sleep Function ─────────────────────────────────────────────
void goToSleep() {
    while (PORTA.IN & PIN2_bm) delay(50); // Wait until PIR goes LOW (no motion)
    delay(100);                            // Small settling delay
    pirTriggered = false;                  // Clear PIR flag
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // Set ATtiny sleep mode to POWER_DOWN
    sleep_enable();                        // Enable sleep
    sei();                                  // Enable global interrupts
    sleep_cpu();                            // Enter sleep mode
    sleep_disable();                        // Wake up: disable sleep
}

void initPlayer(){
    powerOn();                     // Power on DFPlayer
    dfp.begin(Serial, 9600);       // Initialize DFPlayer UART
    delay(50);
    dfp.waitForInit();             // Wait for DFPlayer ready signal
    delay(50);
    dfp.volume(VOLUME);            // Set volume
    delay(50);
}

// ── DFPlayer Power Control ───────────────────────────────────
void powerOn()  { digitalWrite(PIN_POWER, HIGH); }  // Turn on DFPlayer
void powerOff() { digitalWrite(PIN_POWER, LOW);  }  // Turn off DFPlayer

void setup() {
    pinMode(PIN_PIR, INPUT);    // PIR sensor input
    pinMode(PIN_POWER, OUTPUT); // DFPlayer power control

    // Configure PIR pin interrupt: trigger on both rising and falling edges
    // ISR filters only the rising edge for playback
    PORTA.PIN2CTRL = PORT_ISC_RISING_gc;
    initPlayer();
    totalTracks = dfp.trackCount(); // Get total number of tracks
    delay(100);
    dfp.stop();                     // Stop any playback
    powerOff();                     // Turn off DFPlayer to save power

    goToSleep();                    // Enter sleep mode until PIR triggers

}

void loop() {
    if (pirTriggered) {             // Check if PIR triggered
        pirTriggered = false;       // Reset flag
            initPlayer();   
            dfp.playRandom(totalTracks); // Play a random track

            delay(INTERVAL);            // Wait for track playback duration
            dfp.volume(0);
            delay(100);
            dfp.stop();                 // Stop playback
            delay(100);                 // Short delay before power off
            powerOff();                 // Turn off DFPlayer to save power
            Serial.end();
            goToSleep();                // Go back to sleep waiting for PIR
    }
}

