# JTS_DFPlayerTiny

A lightweight Arduino library for the DFPlayer Mini, optimized for the **ATtiny412** (256 bytes RAM, 4 KB Flash). Works without the memory-heavy DFRobotDFPlayerMini library.

## Why JTS_DFPlayerTiny?

The official DFRobotDFPlayerMini library causes a stack overflow on the ATtiny412 if `begin()` is called from within a function. JTS_DFPlayerTiny avoids this by using minimal RAM and direct UART communication with no library overhead.

## Hardware

Tested with:
- **ATtiny412** (megaTinyCore)
- **DFPlayer Mini** (various clones)

### Circuit (ATtiny412)

```
ATtiny412 PA6 (TX) ──[1kΩ]── DFPlayer RX
ATtiny412 PA7 (RX) ────────── DFPlayer TX
ATtiny412 GND      ────────── DFPlayer GND
```

> **Important:** Insert a 1kΩ resistor between the ATtiny TX and DFPlayer RX. Without it, current can flow through the RX pin when the DFPlayer is off, causing malfunction.

## Installation

Copy `JTS_DFPlayerTiny.h` and `JTS_DFPlayerTiny.cpp` from the `src/` folder into the same folder as your `.ino` sketch. No library manager needed.

## Quickstart

```cpp
#include "JTS_DFPlayerTiny.h"

JTS_DFPlayerTiny dfp;

void setup() {
    dfp.begin(Serial, 9600);     // Start Serial
    dfp.waitForInit();            // Wait until DFPlayer is ready
    dfp.volume(20);               // Set volume 0–30
    dfp.play(1);                  // Play track 1
}
```

## API

### Init

```cpp
void begin(Stream &stream, uint32_t baud = 9600);
```
Starts Serial and resets internal variables.

```cpp
bool waitForInit(uint16_t timeoutMs = 3000, uint16_t settleMs = 100);
```
Blocks until the DFPlayer sends the `0x3F` init packet (max `timeoutMs`).
`settleMs`: additional delay after receiving the init packet. Empirical value — may need adjustment depending on clone. Returns `true` if init is successful, `false` on timeout.

### Playback

```cpp
void play(uint16_t track);        // Play track (1-based)
void next();                      // Next track
void previous();                  // Previous track
void stop();                      // Stop
void loopTrack(uint16_t track);   // Loop a track
void loopAll();                   // Loop all tracks
void shuffle();                   // Shuffle all tracks (loop)
void playRandom(uint16_t total);  // Play a random track once
```

`playRandom()` avoids repeating the last track and uses `millis()` as a seed — external events (like a PIR sensor) are time-random, providing a good source of randomness.

### Folders

```cpp
void playFolder(uint8_t folder, uint8_t track); // Folders 01–99, tracks 001–255
void playMP3(uint16_t track);     // From /MP3 folder
void playAdvert(uint16_t track);  // From /ADVERT folder (interrupts, then resumes)
```

### Volume

```cpp
void volume(uint8_t vol);   // 0–30
void volumeUp();            // +1 (max 30)
void volumeDown();          // -1 (min 0)
```

### Info

```cpp
uint16_t trackCount(uint16_t timeoutMs = 2000);
```
Queries the number of MP3 files on the SD card (command `0x48`).
**Note:** Only counts MP3 files, not WAV. For WAV, hardcode `totalTracks`.

### Public Variables

```cpp
uint16_t lastTrack;   // Last played track (used by playRandom)
uint8_t  currentVol;  // Current volume (used by volumeUp/Down)
```

## Examples

| Example | Description |
|---|---|
| `BasicPlayback` | Minimal example, plays two tracks |
| `PIR_Player` | PIR-controlled player without sleep |
| `PIR_Player_Sleep` | PIR-controlled player with POWER_DOWN sleep (~1-3µA -> PIR-Sensor) |

## Notes

- SD card: All files flat in root, no subfolders (except when using `playFolder` / `playMP3` / `playAdvert`)
- Clicking noise on DFPlayer power-up cannot be fully prevented by software — a hardware mod on the 8002 amplifier chip (connect SHUTDOWN pin to BUSY) may help
- Library is designed for ATtiny412 but should work on other AVR chips with hardware UART

## License

GNU GPL

