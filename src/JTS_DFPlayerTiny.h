// ============================================================
//  DFPlayerTiny.h
//  Lightweight DFPlayer Mini Library
//  Optimiert für ATtiny412, funktioniert mit jedem Arduino
//
//  Verwendung:
//    #include "DFPlayerTiny.h"
//    DFPlayerTiny dfp;
//    dfp.begin(Serial, 9600);       // ATtiny412 / Arduino
//    dfp.begin(Serial1, 9600);      // ESP32, Mega etc.
//    dfp.begin(mySoftSerial, 9600); // SoftwareSerial
//    dfp.waitForInit();
//    dfp.volume(20);
//    dfp.play(1);
// ============================================================

#pragma once
#include <Arduino.h>

class JTS_DFPlayerTiny {
public:
    uint16_t lastTrack;   // letzter gespielter Track (für playRandom)
    uint8_t  currentVol;  // aktuelle Lautstärke (für volumeUp/Down)

    // ── Init ─────────────────────────────────────────────────
    // Serial-Stream übergeben und starten
    void begin(Stream &stream, uint32_t baud = 9600);

    // Blockiert bis DFPlayer 0x3F Init-Paket schickt (max. timeoutMs).
    // settleMs: zusätzliche Wartezeit nach dem Init-Paket — je nach Klon
    // nötig damit der Player stabil ist bevor Befehle gesendet werden.
    // Empirischer Wert: 100ms funktioniert meist, bei manchen Klonen mehr.
    // Gibt true zurück wenn Init erfolgreich, false bei Timeout.
    bool waitForInit(uint16_t timeoutMs = 3000, uint16_t settleMs = 100);

    // ── Wiedergabe ───────────────────────────────────────────
    void play(uint16_t track);        // Track abspielen (1-basiert)
    void next();                      // nächster Track
    void previous();                  // voriger Track
    void stop();                      // stopp
    void loopTrack(uint16_t track);   // Track in Dauerschleife
    void loopAll();                   // alle Tracks in Schleife
    void shuffle();                   // alle Tracks zufällig (loop)

    // Einmalig zufälligen Track spielen, kein Repeat des letzten Tracks.
    // Braucht total — vorher trackCount() aufrufen!
    void playRandom(uint16_t total);

    // ── Ordner ───────────────────────────────────────────────
    void playFolder(uint8_t folder, uint8_t track); // Ordner 01-99, Track 001-255
    void playMP3(uint16_t track);     // aus /MP3 Ordner
    void playAdvert(uint16_t track);  // aus /ADVERT Ordner (unterbricht, dann weiter)

    // ── Lautstärke ───────────────────────────────────────────
    void volume(uint8_t vol);         // 0–30
    void volumeUp();                  // +1 (max. 30)
    void volumeDown();                // -1 (min. 0)

    // ── Info ─────────────────────────────────────────────────
    // Anzahl MP3-Dateien auf SD abfragen (nur MP3, keine WAV).
    // Bei WAV-Dateien: totalTracks hardcoden.
    uint16_t trackCount(uint16_t timeoutMs = 2000);

private:
    Stream  *_serial;
    void     sendCmd(uint8_t cmd, uint8_t p1, uint8_t p2);
    uint16_t readResponse(uint16_t timeoutMs);
    void     flushRx();
};
