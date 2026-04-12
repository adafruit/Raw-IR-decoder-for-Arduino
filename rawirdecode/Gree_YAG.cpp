#include <Arduino.h>

// =============================================================================
// Gree YAG Remote Control IR Decoder
// =============================================================================
//
// Protocol: Gree YAG (variant of Gree YAC with additional blocks)
// Modulation: 38 kHz, space-encoded
//
// IR Timings:
//   Header mark:  ~9000 us  (some units use ~6000/~8250)
//   Header space: ~4450 us  (some units use ~3000/~3750)
//   Bit mark:     ~650 us
//   One space:    ~1620 us
//   Zero space:   ~520 us
//   Mid-msg gap:  ~28000 us (pause between blocks)
//
// Recommended decoder thresholds (midpoints):
//   MARK_THRESHOLD_BIT_HEADER    = 4450
//   SPACE_THRESHOLD_ZERO_ONE     = 1080
//   SPACE_THRESHOLD_ONE_HEADER   = 2680
//   SPACE_THRESHOLD_HEADER_PAUSE = 15000
//
// =============================================================================
// FRAME STRUCTURE
// =============================================================================
//
// Each block = Hh [byte0..3] 010 W [byte4..7]
//   Hh = header mark + space
//   010 = 3-bit block footer (always 0b010)
//   W = mid-message gap (~28ms)
//
// Bits are transmitted LSB-first within each byte.
//
// Frame types by pulse count:
//   19  = iFeel only (2 bytes: temperature + 0xA5 identifier)
//   213 = 3 blocks, no iFeel (normal command, 24 bytes)
//   232 = 3 blocks + iFeel (normal command, 26 bytes)
//   284 = 4 blocks, no iFeel (timer ON command, 32 bytes)
//   303 = 4 blocks + iFeel (timer ON command, 34 bytes)
//
// Normal command (3 blocks):
//   Block 1 [bytes 0-7]:   Main settings     (block ID 0x5 in byte3[7:4])
//   Block 2 [bytes 8-15]:  Mirror / extended  (block ID 0x7 in byte11[7:4])
//   Block 3 [bytes 16-23]: Padding            (block ID 0xA in byte19[7:4])
//   iFeel   [bytes 24-25]: Room temp + 0xA5   (optional)
//
// Timer ON command (4 blocks):
//   Block 1 [bytes 0-7]:   Main settings      (block ID 0x5 in byte3[7:4])
//   Block 2 [bytes 8-15]:  Timer data block    (block ID 0x6 in byte11[7:4])
//   Block 3 [bytes 16-23]: Mirror / extended   (block ID 0x7 in byte19[7:4])
//   Block 4 [bytes 24-31]: Padding             (block ID 0xA in byte27[7:4])
//   iFeel   [bytes 32-33]: Room temp + 0xA5    (optional)
//
// Timer cancel: sends a normal 3-block command (timer bits cleared).
//
// =============================================================================
// CHECKSUM (per 8-byte block)
// =============================================================================
//
//   checksum = ( byte0[3:0] + byte1[3:0] + byte2[3:0] + byte3[3:0]
//              + byte4[7:4] + byte5[7:4] + byte6[7:4] + 0x0A ) & 0x0F
//   Stored in byte7[7:4]
//
// =============================================================================
// BYTE MAP
// =============================================================================
//
// BLOCK 1 (bytes 0-7) — Main settings:
//
//   Byte 0: [2:0] Operating mode
//                  0=Auto, 1=Cool, 2=Dry, 3=Fan, 4=Heat
//           [3]   Power (0=Off, 1=On)
//           [5:4] Fan speed base (0=Auto, 1=Low, 2=Med, 3=High/ext)
//                  When 3, actual speed 3-5 is in byte14[6:4]
//           [6]   Swing auto (0=Off, 1=On)
//           [7]   Sleep mode 1 (0=Off, 1=On)
//
//   Byte 1: [3:0] Temperature = value + 16 (range 16-30 C)
//           [7:4] Timer flag
//                  0x0 = no timer
//                  0xA = timer ON active
//                  0xB = timer ON active (alternate, possibly >24h related)
//                  (Timer OFF / schedule shutdown: not yet captured)
//
//   Byte 2: [3:0] Timer-related (non-zero only during timer ON commands,
//                  partially correlated with target hour, encoding unclear)
//           [4]   Turbo (0=Off, 1=On)
//           [5]   Light / display LED (0=Off, 1=On)
//           [6]   Health / ionizer (0=Off, 1=On)
//           [7]   XFan / Blow (0=Off, 1=On, keeps fan running after off)
//
//   Byte 3: [3:0] Reserved (always 0x0 in captures)
//           [7:4] Block ID: 0x5
//
//   Byte 4: [3:0] Vertical vane position
//                  0x0=Stop, 0x1=Sweep 1-5, 0x2=Up(1), 0x3=High(2),
//                  0x4=Center(3), 0x5=Low(4), 0x6=Down(5),
//                  0x7=Sweep 3-5, 0x9=Sweep 2-4, 0xB=Sweep 1-3
//           [7:4] Horizontal vane position
//                  0x0=Stop, 0x1=Sweep 1-5, 0x2=Far left(1), 0x3=Left(2),
//                  0x4=Center(3), 0x5=Right(4), 0x6=Far right(5),
//                  0xC=Left+Right(1+5), 0xD=Sweep to center
//
//   Byte 5: [1:0] Display temperature source
//                  0/1=Set temp, 2=Indoor temp, 3=Outdoor temp
//                  (Outdoor not available on all models)
//           [2]   iFeel enable (0=Off, 1=On)
//                  When on, remote sends room temp every 10 min
//                  and 200ms after each button press
//           [7:3] Reserved
//
//   Byte 6: [7:0] Reserved (always 0x00 in captures)
//
//   Byte 7: [1:0] Reserved
//           [2]   Eco mode / 8C heating (0=Off, 1=On)
//           [3]   Reserved
//           [7:4] Checksum
//
// BLOCK 2 — Normal mode (bytes 8-15):
//   Byte 8:  Mirror of byte 0 (same mode/power/fan/swing/sleep)
//   Byte 9:  Mirror of byte 1 low nibble (same temperature)
//            Byte 9 high nibble = 0x0 (no timer flag in block 2)
//   Byte 10: Mirror of byte 2 high nibble (turbo/light/health/xfan)
//            Byte 10 low nibble = 0x0
//   Byte 11: Block ID 0x7 in [7:4], [3:0] = 0x0
//   Byte 12: [0]   Sleep mode 2 (0=Off, 1=On)
//            [7:1] Reserved
//   Byte 13: [7:0] Reserved (0x00)
//   Byte 14: [3:0] Reserved
//            [6:4] Fan speed extended (0-5 mapping: 0=auto,1-5=speeds)
//            [7]   Reserved
//   Byte 15: [7:4] Checksum, [3:0] reserved
//
// BLOCK 2 — Timer ON mode (bytes 8-15):
//   Byte 8-10: Mirror of bytes 0-2 (with timer bits)
//   Byte 11:   Block ID 0x6 in [7:4]
//   Byte 12:   Timer countdown low byte (minutes, bits [7:0])
//   Byte 13:   [2:0] Timer countdown high bits (minutes, bits [10:8])
//              [3]   Timer active flag (always 1 when timer set)
//              [7:4] Reserved
//              Formula: countdown_minutes = byte12 | ((byte13 & 0x07) << 8)
//              Maximum representable: 2047 minutes (~34 hours)
//   Byte 14:   [6:4] Fan speed extended (same as normal mode)
//   Byte 15:   [7:4] Checksum
//
// BLOCK 3 — Timer ON mode (bytes 16-23):
//   Mirrors block 1 settings with block ID 0x7 in byte19[7:4]
//
// PADDING BLOCK (last data block before iFeel):
//   Always: 00 00 00 A0 00 00 00 A0
//   Block ID 0xA in byte [3] upper nibble of each half
//
// iFEEL FRAME (2 bytes, appended after last block):
//   Byte 0: Room temperature in degrees C (raw value, e.g. 0x1A = 26 C)
//   Byte 1: Always 0xA5 (identifier/checksum)
//   Sent as standalone 19-pulse frame every 10 minutes when iFeel is on,
//   and 200ms after each button press.
//
// =============================================================================
// NOT DECODED (insufficient data or out of scope)
// =============================================================================
//
// - Timer OFF (schedule AC shutdown): not yet captured. Likely uses same
//   countdown formula but may use different byte1[7:4] value to distinguish
//   from Timer ON.
// - Sleep 3 (DIY 8-point temperature curve): uses a different frame format
//   with more bytes. Requires multi-step UI interaction to set.
// - WiFi button: toggles WiFi icon on remote. MODE+WiFi held 1s resets
//   WiFi module to factory defaults. No IR captures taken.
// - byte1[7:4] = 0xA vs 0xB during timer: both indicate timer ON active,
//   distinguishing condition not yet determined.
// - byte2[3:0] during timer: non-zero, partially correlates with target
//   hour but exact encoding not confirmed across all samples.
//
// =============================================================================

bool decodeGree_YAG(byte *bytes, int pulseCount)
{
  // ---- iFeel-only short frame (19 pulses, 2 bytes) ----
  if (pulseCount == 19) {
    Serial.println(F("Gree YAG: I-Feel temperature frame"));
    Serial.print(F("  Room Temperature: "));
    Serial.print(bytes[0]);
    Serial.print(F(" C"));
    if (bytes[1] == 0xA5) {
      Serial.println(F(" (ID: 0xA5 OK)"));
    } else {
      Serial.print(F(" (ID: 0x"));
      Serial.print(bytes[1], HEX);
      Serial.println(F(" UNEXPECTED)"));
    }
    return true;
  }

  // ---- Determine frame type ----
  int numBlocks;
  bool hasIFeel;
  int iFeelByteIdx;

  switch (pulseCount) {
    case 213: numBlocks = 3; hasIFeel = false; iFeelByteIdx = -1; break;
    case 232: numBlocks = 3; hasIFeel = true;  iFeelByteIdx = 24; break;
    case 284: numBlocks = 4; hasIFeel = false; iFeelByteIdx = -1; break;
    case 303: numBlocks = 4; hasIFeel = true;  iFeelByteIdx = 32; break;
    default:  return false;
  }

  bool isTimerCmd = (numBlocks == 4);

  Serial.print(F("Gree YAG protocol"));
  if (isTimerCmd) Serial.print(F(" (TIMER CMD)"));
  Serial.println();

  // ---- Raw byte dump ----
  for (int blk = 0; blk < numBlocks; blk++) {
    int base = blk * 8;
    Serial.print(F("  Block "));
    Serial.print(blk + 1);
    Serial.print(F(" [ID=0x"));
    Serial.print((bytes[base + 3] >> 4) & 0x0F, HEX);
    Serial.print(F("]: "));
    for (int i = 0; i < 8; i++) {
      if (bytes[base + i] < 0x10) Serial.print('0');
      Serial.print(bytes[base + i], HEX);
      Serial.print(' ');
    }
    Serial.println();
  }
  if (hasIFeel) {
    Serial.print(F("  iFeel bytes: "));
    if (bytes[iFeelByteIdx] < 0x10) Serial.print('0');
    Serial.print(bytes[iFeelByteIdx], HEX);
    Serial.print(' ');
    if (bytes[iFeelByteIdx + 1] < 0x10) Serial.print('0');
    Serial.println(bytes[iFeelByteIdx + 1], HEX);
  }

  // ---- Checksums (all blocks) ----
  bool allChecksumsOk = true;
  for (int blk = 0; blk < numBlocks; blk++) {
    int base = blk * 8;
    uint8_t sum = (
      (bytes[base + 0] & 0x0F) +
      (bytes[base + 1] & 0x0F) +
      (bytes[base + 2] & 0x0F) +
      (bytes[base + 3] & 0x0F) +
      ((bytes[base + 4] & 0xF0) >> 4) +
      ((bytes[base + 5] & 0xF0) >> 4) +
      ((bytes[base + 6] & 0xF0) >> 4) +
      0x0A) & 0x0F;
    uint8_t expected = bytes[base + 7] >> 4;

    Serial.print(F("  Checksum block "));
    Serial.print(blk + 1);
    if (sum == expected) {
      Serial.println(F(": OK"));
    } else {
      Serial.print(F(": FAIL (calc=0x"));
      Serial.print(sum, HEX);
      Serial.print(F(" exp=0x"));
      Serial.print(expected, HEX);
      Serial.println(')');
      allChecksumsOk = false;
    }
  }

  // ---- Power ----
  Serial.print(F("  Power:       "));
  Serial.println((bytes[0] & 0x08) ? F("ON") : F("OFF"));

  // ---- Operating Mode ----
  Serial.print(F("  Mode:        "));
  switch (bytes[0] & 0x07) {
    case 0x00: Serial.println(F("AUTO")); break;
    case 0x01: Serial.println(F("COOL")); break;
    case 0x02: Serial.println(F("DRY"));  break;
    case 0x03: Serial.println(F("FAN"));  break;
    case 0x04: Serial.println(F("HEAT")); break;
    default:
      Serial.print(F("UNKNOWN (0x"));
      Serial.print(bytes[0] & 0x07, HEX);
      Serial.println(')');
      break;
  }

  // ---- Temperature ----
  uint8_t temp = (bytes[1] & 0x0F) + 16;
  Serial.print(F("  Temperature: "));
  Serial.print(temp);
  Serial.println(F(" C"));

  // ---- Fan Speed ----
  Serial.print(F("  Fan:         "));
  uint8_t fanBase = (bytes[0] & 0x30) >> 4;
  uint8_t fanExt = (bytes[14] & 0x70) >> 4;
  switch (fanBase) {
    case 0: Serial.println(F("AUTO")); break;
    case 1: Serial.println(F("1 (Low)"));  break;
    case 2: Serial.println(F("2 (Med)"));  break;
    case 3:
      switch (fanExt) {
        case 3: Serial.println(F("3 (High)"));    break;
        case 4: Serial.println(F("4 (Higher)"));  break;
        case 5: Serial.println(F("5 (Highest)")); break;
        default:
          Serial.print(F("3+ (ext="));
          Serial.print(fanExt);
          Serial.println(')');
          break;
      }
      break;
  }

  // ---- Eco ----
  Serial.print(F("  Eco:         "));
  Serial.println((bytes[7] & 0x04) ? F("ON (8C heating)") : F("OFF"));

  // ---- Turbo ----
  Serial.print(F("  Turbo:       "));
  Serial.println((bytes[2] & 0x10) ? F("ON") : F("OFF"));

  // ---- Light ----
  Serial.print(F("  Light:       "));
  Serial.println((bytes[2] & 0x20) ? F("ON") : F("OFF"));

  // ---- Health / Ionizer ----
  Serial.print(F("  Health:      "));
  Serial.println((bytes[2] & 0x40) ? F("ON") : F("OFF"));

  // ---- XFan / Blow ----
  Serial.print(F("  XFan:        "));
  Serial.println((bytes[2] & 0x80) ? F("ON") : F("OFF"));

  // ---- Sleep Mode ----
  // Sleep 1: byte0 bit 7
  // Sleep 2: byte12 bit 0 (only valid in non-timer frames)
  // Sleep 3: uses different frame format (not decoded)
  Serial.print(F("  Sleep:       "));
  if (bytes[0] & 0x80) {
    Serial.println(F("MODE 1"));
  } else if (!isTimerCmd && (bytes[12] & 0x01)) {
    Serial.println(F("MODE 2"));
  } else {
    Serial.println(F("OFF"));
  }

  // ---- Swing Auto ----
  Serial.print(F("  Swing:       "));
  Serial.println((bytes[0] & 0x40) ? F("AUTO") : F("OFF"));

  // ---- Vertical Vane ----
  Serial.print(F("  Vert. Vane:  "));
  switch (bytes[4] & 0x0F) {
    case 0x00: Serial.println(F("STOP"));          break;
    case 0x01: Serial.println(F("SWEEP 1-5"));     break;
    case 0x02: Serial.println(F("UP (1)"));        break;
    case 0x03: Serial.println(F("HIGH (2)"));      break;
    case 0x04: Serial.println(F("CENTER (3)"));    break;
    case 0x05: Serial.println(F("LOW (4)"));       break;
    case 0x06: Serial.println(F("DOWN (5)"));      break;
    case 0x07: Serial.println(F("SWEEP 3-5"));     break;
    case 0x09: Serial.println(F("SWEEP 2-4"));     break;
    case 0x0B: Serial.println(F("SWEEP 1-3"));     break;
    default:
      Serial.print(F("UNKNOWN (0x"));
      Serial.print(bytes[4] & 0x0F, HEX);
      Serial.println(')');
      break;
  }

  // ---- Horizontal Vane ----
  Serial.print(F("  Horiz. Vane: "));
  switch (bytes[4] & 0xF0) {
    case 0x00: Serial.println(F("STOP"));              break;
    case 0x10: Serial.println(F("SWEEP 1-5"));         break;
    case 0x20: Serial.println(F("FAR LEFT (1)"));      break;
    case 0x30: Serial.println(F("LEFT (2)"));          break;
    case 0x40: Serial.println(F("CENTER (3)"));        break;
    case 0x50: Serial.println(F("RIGHT (4)"));         break;
    case 0x60: Serial.println(F("FAR RIGHT (5)"));     break;
    case 0xC0: Serial.println(F("LEFT+RIGHT (1+5)"));  break;
    case 0xD0: Serial.println(F("SWEEP TO CENTER"));   break;
    default:
      Serial.print(F("UNKNOWN (0x"));
      Serial.print((bytes[4] & 0xF0) >> 4, HEX);
      Serial.println(')');
      break;
  }

  // ---- Display Temperature Source ----
  Serial.print(F("  Display:     "));
  switch (bytes[5] & 0x03) {
    case 0x00:
    case 0x01: Serial.println(F("SET TEMP"));          break;
    case 0x02: Serial.println(F("INDOOR TEMP"));       break;
    case 0x03: Serial.println(F("OUTDOOR TEMP"));      break;
  }

  // ---- iFeel ----
  bool iFeelOn = bytes[5] & 0x04;
  Serial.print(F("  iFeel:       "));
  Serial.println(iFeelOn ? F("ON") : F("OFF"));

  if (hasIFeel) {
    Serial.print(F("  iFeel Temp:  "));
    Serial.print(bytes[iFeelByteIdx]);
    Serial.print(F(" C"));
    if (bytes[iFeelByteIdx + 1] != 0xA5) {
      Serial.print(F(" (ID: 0x"));
      Serial.print(bytes[iFeelByteIdx + 1], HEX);
      Serial.print(F(" UNEXPECTED)"));
    }
    Serial.println();
  }

  // ---- Timer ON ----
  // Timer ON is indicated by:
  //   - 4-block frame (284 or 303 pulses)
  //   - byte1[7:4] = 0xA or 0xB
  //   - byte13 bit 3 set
  //   - Countdown in minutes = byte12 | ((byte13 & 0x07) << 8)
  // Timer cancel is a normal 3-block frame with byte1[7:4] = 0x0.
  // Timer OFF (schedule shutdown) not yet captured.
  uint8_t timerNibble = (bytes[1] >> 4) & 0x0F;
  bool timerActive = (timerNibble >= 0x0A);

  Serial.print(F("  Timer ON:    "));
  if (isTimerCmd && timerActive) {
    uint16_t countdownMin = (uint16_t)bytes[12] | (((uint16_t)bytes[13] & 0x07) << 8);
    uint16_t hours = countdownMin / 60;
    uint16_t mins  = countdownMin % 60;

    Serial.print(F("ACTIVE (countdown "));
    Serial.print(hours);
    Serial.print(F("h"));
    if (mins < 10) Serial.print('0');
    Serial.print(mins);
    Serial.print(F("m = "));
    Serial.print(countdownMin);
    Serial.println(F(" min)"));

    // Debug fields for continued reverse engineering
    Serial.print(F("    byte1[7:4]:  0x"));
    Serial.println(timerNibble, HEX);
    Serial.print(F("    byte2[3:0]:  0x"));
    Serial.println(bytes[2] & 0x0F, HEX);
    Serial.print(F("    countdown:   byte12=0x"));
    Serial.print(bytes[12], HEX);
    Serial.print(F(" byte13=0x"));
    Serial.print(bytes[13], HEX);
    Serial.print(F(" (hi="));
    Serial.print(bytes[13] & 0x07);
    Serial.print(F(" flag="));
    Serial.print((bytes[13] & 0x08) ? '1' : '0');
    Serial.println(')');
  } else {
    Serial.println(F("OFF"));
  }

  // ---- Consistency checks ----
  if (bytes[0] != bytes[8]) {
    Serial.print(F("  NOTE: byte0 (0x"));
    Serial.print(bytes[0], HEX);
    Serial.print(F(") != byte8 (0x"));
    Serial.print(bytes[8], HEX);
    Serial.println(F(")"));
  }
  if ((bytes[1] & 0x0F) != (bytes[9] & 0x0F)) {
    Serial.print(F("  NOTE: temp block1="));
    Serial.print((bytes[1] & 0x0F) + 16);
    Serial.print(F(" != block2="));
    Serial.println((bytes[9] & 0x0F) + 16);
  }

  return true;
}