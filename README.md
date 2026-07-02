# Strain Gauge Readout — Nucleo-F446ZE + HX711

Firmware that reads a quarter-bridge strain gauge (Wheatstone configuration)
through an HX711 24-bit ADC and prints **calibrated weight** over the Nucleo's
USB serial port.

- **Board:** ST Nucleo-F446ZE
- **Toolchain:** PlatformIO + Arduino framework (STM32duino)
- **Library:** [`bogde/HX711`](https://github.com/bogde/HX711)

## Wiring

The HX711 is bit-banged GPIO, not the SPI peripheral. `SCK` is a clock the MCU
drives; `DT`/`DOUT` is the data line the MCU reads.

| HX711 pin | Wire to | Nucleo pin | STM32 pin | Direction (MCU) |
|-----------|---------|------------|-----------|-----------------|
| VCC       | 3V3     | 3V3        | —         | power           |
| GND       | GND     | GND        | —         | power           |
| SCK       | D13     | D13        | PA5       | clock out       |
| DT/DOUT   | D12     | D12        | PA6       | data in         |

The strain gauge bridge connects to the HX711's `E+/E-` (excitation) and
`A+/A-` (channel A, gain 128) inputs.

> 3V3 excitation is fine (HX711 rated 2.6–5.5 V) and keeps `DOUT` at clean 3.3 V
> logic. If resolution looks poor, try moving VCC to the Nucleo **5V** pin.

## Build / flash / monitor

```bash
pio run                       # compile (pulls bogde/HX711 automatically)
pio run -t upload             # flash via onboard ST-Link
pio device monitor -b 115200  # view output (port auto-detected, e.g. /dev/ttyACM0)
```

### Ubuntu one-time setup

```bash
sudo usermod -a -G dialout $USER   # serial access; log out/in after
```

If upload can't find the ST-Link probe, install PlatformIO's udev rules
(`99-platformio-udev.rules`) per the
[PlatformIO docs](https://docs.platformio.org/en/latest/core/installation/udev-rules.html).

## Serial calibration

Interactive keys in the serial monitor:

| Key | Action                         |
|-----|--------------------------------|
| `t` | tare (zero with no load)       |
| `a` | calibration_factor += step     |
| `z` | calibration_factor -= step     |
| `p` | print current factor           |

Procedure:

1. Flash and open the monitor. With no load, readings hover near 0.
2. Press `t` to tare.
3. Place a **known mass** (e.g. 100 g) on the gauge.
4. Press `a`/`z` until the printed `weight` matches the known mass.
5. Press `p`, note the value, and paste it into the `calibration_factor`
   initializer in `src/main.cpp` so it persists across resets.

## Troubleshooting

- **"HX711 not ready"** repeating → DT/SCK wiring or power issue. First thing to
  try: swap the D12/D13 assignments in `src/main.cpp`.
- **Readings stuck at a rail / no change under load** → check bridge wiring to
  `A+/A-` and `E+/E-`, and confirm a stable VCC.
