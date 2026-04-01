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
//  PA3 (Pin 7) = Transistor Gate (DFPlayer Power)
//  PA6 (Pin 2) = UART TX → DFPlayer RX (1kΩ in series)
//  PA7 (Pin 3) = UART RX ← DFPlayer TX
// ============================================================

#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "JTS_DFPlayerTiny.h"

#define PIN_POWER  PIN_PA3  // Pin controlling DFPlayer power via transistor
#define PIN_PIR    PIN_PA1  // Pin connected to PIR sensor output

const uint16_t INTERVAL = 5000;  // Time in ms to keep player on after track finishes
const uint8_t  VOLUME   = 30;    // Volume level for playback (0-30)

JTS_DFPlayerTiny dfp;             // DFPlayer object
volatile bool pirTriggered = false; // Flag set by PIR interrupt
uint16_t totalTracks       = 0;    // Total number of tracks on SD card

// ── PIR Interrupt Service Routine ─────────────────────────────
ISR(PORTA_PORT_vect) {
    // Check if the interrupt is from PIR pin (PA1)
    if (PORTA.INTFLAGS & PIN1_bm) {
        PORTA.INTFLAGS = PIN1_bm; // Clear interrupt flag
        if (PORTA.IN & PIN1_bm) { // Rising edge detected
            pirTriggered = true;  // Set flag to trigger playback
        }
    }
}

// ── Sleep Function ─────────────────────────────────────────────
void goToSleep() {
    while (PORTA.IN & PIN1_bm) delay(50); // Wait until PIR goes LOW (no motion)
    delay(100);                            // Small settling delay
    pirTriggered = false;                  // Clear PIR flag
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // Set ATtiny sleep mode to POWER_DOWN
    sleep_enable();                        // Enable sleep
    sei();                                  // Enable global interrupts
    sleep_cpu();                            // Enter sleep mode
    sleep_disable();                        // Wake up: disable sleep
}

// ── DFPlayer Power Control ───────────────────────────────────
void powerOn()  { digitalWrite(PIN_POWER, HIGH); }  // Turn on DFPlayer
void powerOff() { digitalWrite(PIN_POWER, LOW);  }  // Turn off DFPlayer

void setup() {
    pinMode(PIN_PIR, INPUT);    // PIR sensor input
    pinMode(PIN_POWER, OUTPUT); // DFPlayer power control

    // Configure PIR pin interrupt: trigger on both rising and falling edges
    // ISR filters only the rising edge for playback
    PORTA.PIN1CTRL = PORT_ISC_BOTHEDGES_gc;

    powerOn();                     // Power on DFPlayer
    dfp.begin(Serial, 9600);      // Initialize DFPlayer UART
    delay(100);
    dfp.waitForInit();             // Wait for DFPlayer ready signal
    dfp.volume(VOLUME);            // Set playback volume
    totalTracks = dfp.trackCount(); // Get total number of tracks
    dfp.stop();                     // Stop any playback
    powerOff();                     // Turn off DFPlayer to save power

    goToSleep();                    // Enter sleep mode until PIR triggers

}

void loop() {
    if (pirTriggered) {             // Check if PIR triggered
        pirTriggered = false;       // Reset flag

            powerOn();                  // Power on DFPlayer
            delay(100);                 // Allow time to stabilize
            dfp.waitForInit();          // Wait for DFPlayer ready
            dfp.volume(VOLUME);         // Set volume
            dfp.playRandom(totalTracks); // Play a random track

            delay(INTERVAL);            // Wait for track playback duration

            dfp.stop();                 // Stop playback
            delay(200);                 // Short delay before power off
            powerOff();                 // Turn off DFPlayer to save power

            goToSleep();                // Go back to sleep waiting for PIR
    }
}
