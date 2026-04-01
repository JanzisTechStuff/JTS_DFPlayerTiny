// ============================================================
//  BasicPlayback
//  Minimales Beispiel: DFPlayer Mini mit DFPlayerTiny Library
//
//  Spielt Track 1 ab, dann nach 5 Sekunden Track 2.
//
//  Pin-Belegung (ATtiny412):
//  PA6 (Pin 2) = UART TX → DFPlayer RX (1kΩ in Serie!)
//  PA7 (Pin 3) = UART RX ← DFPlayer TX
//  GND         = GND
//  VCC (5V)    = VCC (DFPlayer dauerhaft versorgt)
// ============================================================

#include "JTS_DFPlayerTiny.h"

JTS_DFPlayerTiny dfp;

void setup() {
    dfp.begin(Serial, 9600);
    dfp.waitForInit();   // warten bis DFPlayer bereit
    dfp.volume(20);      // Lautstärke 0–30
    dfp.play(1);         // Track 1 abspielen
    delay(5000);
    dfp.play(2);         // Track 2 abspielen
}

void loop() {
    // nichts
}
