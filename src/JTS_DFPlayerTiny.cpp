// ============================================================
//  DFPlayerTiny.cpp
//  Lightweight DFPlayer Mini Library
// ============================================================

#include "JTS_DFPlayerTiny.h"

// ── Internes: Kommando senden ─────────────────────────────────
void JTS_DFPlayerTiny::sendCmd(uint8_t cmd, uint8_t p1, uint8_t p2) {
    uint16_t cs = 0xFFFF - (0xFF + 0x06 + cmd + 0x00 + p1 + p2) + 1;
    uint8_t buf[10] = {
        0x7E, 0xFF, 0x06, cmd, 0x00,
        p1, p2,
        (uint8_t)(cs >> 8), (uint8_t)(cs & 0xFF),
        0xEF
    };
    for (uint8_t i = 0; i < 10; i++) _serial->write(buf[i]);
}

// ── Internes: RX leeren ───────────────────────────────────────
void JTS_DFPlayerTiny::flushRx() {
    while (_serial->available()) _serial->read();
}

// ── Internes: Antwort lesen ───────────────────────────────────
uint16_t JTS_DFPlayerTiny::readResponse(uint16_t timeoutMs) {
    uint8_t buf[10];
    unsigned long start = millis();
    while (millis() - start < timeoutMs) {
        if (_serial->available() >= 10) {
            for (uint8_t i = 0; i < 10; i++) buf[i] = _serial->read();
            if (buf[0] == 0x7E && buf[9] == 0xEF) {
                return ((uint16_t)buf[5] << 8) | buf[6];
            }
        }
    }
    return 0;
}

// ── Public: begin ─────────────────────────────────────────────
void JTS_DFPlayerTiny::begin(Stream &stream, uint32_t baud) {
    _serial    = &stream;
    lastTrack  = 0;
    currentVol = 0;
    // HardwareSerial initialisieren falls möglich
    if (HardwareSerial *hs = reinterpret_cast<HardwareSerial*>(&stream)) {
        hs->begin(baud);
    }
}

// ── Public: waitForInit ───────────────────────────────────────
bool JTS_DFPlayerTiny::waitForInit(uint16_t timeoutMs, uint16_t settleMs) {
    uint8_t buf[10];
    unsigned long start = millis();
    while (millis() - start < timeoutMs) {
        if (_serial->available() >= 10) {
            for (uint8_t i = 0; i < 10; i++) buf[i] = _serial->read();
            if (buf[0] == 0x7E && buf[3] == 0x3F) {
                if (settleMs > 0) delay(settleMs);
                return true;
            }
        }
    }
    if (settleMs > 0) delay(settleMs);
    return false;
}

// ── Public: play ──────────────────────────────────────────────
void JTS_DFPlayerTiny::play(uint16_t track) {
    lastTrack = track;
    sendCmd(0x03, (uint8_t)(track >> 8), (uint8_t)(track & 0xFF));
}

// ── Public: next ──────────────────────────────────────────────
void JTS_DFPlayerTiny::next() {
    sendCmd(0x01, 0x00, 0x00);
}

// ── Public: previous ──────────────────────────────────────────
void JTS_DFPlayerTiny::previous() {
    sendCmd(0x02, 0x00, 0x00);
}

// ── Public: stop ──────────────────────────────────────────────
void JTS_DFPlayerTiny::stop() {
    sendCmd(0x16, 0x00, 0x00);
    delay(100);
}

// ── Public: loopTrack ─────────────────────────────────────────
void JTS_DFPlayerTiny::loopTrack(uint16_t track) {
    lastTrack = track;
    sendCmd(0x08, (uint8_t)(track >> 8), (uint8_t)(track & 0xFF));
}

// ── Public: loopAll ───────────────────────────────────────────
void JTS_DFPlayerTiny::loopAll() {
    sendCmd(0x11, 0x00, 0x01);
}

// ── Public: shuffle ───────────────────────────────────────────
void JTS_DFPlayerTiny::shuffle() {
    sendCmd(0x18, 0x00, 0x00);
}

// ── Public: playRandom ────────────────────────────────────────
void JTS_DFPlayerTiny::playRandom(uint16_t total) {
    if (total == 0) return;
    if (total == 1) { play(1); return; }
    randomSeed(millis());
    uint16_t track;
    uint8_t attempts = 0;
    do {
        track = (uint16_t)random(1, total + 1);
        attempts++;
    } while (track == lastTrack && attempts < 10);
    play(track);
}

// ── Public: playFolder ────────────────────────────────────────
void JTS_DFPlayerTiny::playFolder(uint8_t folder, uint8_t track) {
    sendCmd(0x0F, folder, track);
}

// ── Public: playMP3 ───────────────────────────────────────────
void JTS_DFPlayerTiny::playMP3(uint16_t track) {
    sendCmd(0x12, (uint8_t)(track >> 8), (uint8_t)(track & 0xFF));
}

// ── Public: playAdvert ────────────────────────────────────────
void JTS_DFPlayerTiny::playAdvert(uint16_t track) {
    sendCmd(0x13, (uint8_t)(track >> 8), (uint8_t)(track & 0xFF));
}

// ── Public: volume ────────────────────────────────────────────
void JTS_DFPlayerTiny::volume(uint8_t vol) {
    if (vol > 30) vol = 30;
    currentVol = vol;
    sendCmd(0x06, 0x00, vol);
    delay(50);
}

// ── Public: volumeUp ──────────────────────────────────────────
void JTS_DFPlayerTiny::volumeUp() {
    if (currentVol < 30) volume(currentVol + 1);
}

// ── Public: volumeDown ────────────────────────────────────────
void JTS_DFPlayerTiny::volumeDown() {
    if (currentVol > 0) volume(currentVol - 1);
}

// ── Public: trackCount ────────────────────────────────────────
uint16_t JTS_DFPlayerTiny::trackCount(uint16_t timeoutMs) {
    flushRx();
    delay(100);
    sendCmd(0x48, 0x00, 0x00);  // 0x48 = Anzahl MP3-Dateien auf SD
    uint16_t count = readResponse(timeoutMs);
    return (count > 0 && count < 1000) ? count : 0;
}
