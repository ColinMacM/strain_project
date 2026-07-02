// Strain gauge readout — Nucleo-F446ZE + HX711
//
// Reads a quarter-bridge strain gauge through an HX711 24-bit ADC and prints
// calibrated weight over the ST-Link USB virtual COM port (/dev/ttyACM0 on
// Ubuntu). The HX711 is bit-banged GPIO, not the SPI peripheral — SCK is a
// clock the MCU drives, DT/DOUT is the data line the MCU reads.
//
// Wiring (HX711 -> Nucleo):
//   VCC -> 3V3
//   GND -> GND
//   SCK -> D13 (PA5)   clock out from MCU
//   DT  -> D12 (PA6)   data in  to  MCU
//
// Serial calibration keys:
//   t = tare (zero with no load)
//   a = calibration_factor += STEP
//   z = calibration_factor -= STEP
//   p = print current calibration_factor

#include <Arduino.h>
#include "HX711.h"

const int HX711_DOUT = D13;  // DT  -> PA5
const int HX711_SCK  = D12;  // SCK -> PA6

// Counts-per-unit divisor. Start with a guess and refine using a known mass
// (see calibration procedure in README), then paste the final value here so it
// persists across resets.
float calibration_factor = 1000.0f;

// How much each 'a'/'z' keypress nudges the calibration factor.
const float CALIB_STEP = 10.0f;

HX711 scale;

void setup() {
  Serial.begin(115200);
  while (!Serial) { /* wait for VCP on boards that need it */ }

  scale.begin(HX711_DOUT, HX711_SCK);  // (DOUT, SCK) order matters
  scale.set_scale(calibration_factor);
  scale.tare();                        // zero with no load on the gauge

  Serial.println();
  Serial.println("HX711 ready.");
  Serial.println("Keys: t=tare  a=factor+  z=factor-  p=print factor");
  Serial.print("Starting calibration_factor = ");
  Serial.println(calibration_factor, 3);
}

void handleSerialCommand() {
  if (!Serial.available()) return;

  char c = Serial.read();
  switch (c) {
    case 't':
      scale.tare();
      Serial.println("tared.");
      break;
    case 'a':
      calibration_factor += CALIB_STEP;
      scale.set_scale(calibration_factor);
      break;
    case 'z':
      calibration_factor -= CALIB_STEP;
      scale.set_scale(calibration_factor);
      break;
    case 'p':
      Serial.print("calibration_factor = ");
      Serial.println(calibration_factor, 3);
      break;
    default:
      break;  // ignore newlines and unknown keys
  }
}

void loop() {
  if (scale.is_ready()) {
    float weight = scale.get_units(10);  // averaged over 10 samples
    Serial.print("weight: ");
    Serial.println(weight, 3);
  } else {
    // HX711 not responding — almost always a DOUT/SCK wiring or power issue.
    Serial.println("HX711 not ready (check DT/SCK wiring and VCC).");
    delay(500);
  }

  handleSerialCommand();
}
