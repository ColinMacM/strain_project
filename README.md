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
| DT/DOUT   | D13     | D13        | PA5       | data in         |
| SCK       | D12     | D12        | PA6       | clock out       |

Pin roles must match `src/main.cpp` (`HX711_DOUT = D13`, `HX711_SCK = D12`). If
you see repeated `HX711 not ready`, DT and SCK are swapped — either fix the wires
or swap those two constants and re-flash.

The strain gauge bridge connects to the HX711's `E+/E-` (excitation) and
`A+/A-` (channel A, gain 128) inputs. `E+`–`E-` should read ~2.7–3.3 V once
powered; `A+`–`A-` must rest within the HX711's ±20 mV window (near 0 V for a
balanced bridge) or the reading rails.

> 3V3 excitation is fine (HX711 rated 2.6–5.5 V). If resolution looks poor you
> can move VCC to the Nucleo **5V** pin — but note `DT` is on **PA5**, which is
> **not** 5 V-tolerant. At 5 V, first move `DT` to a 5 V-tolerant pin (e.g.
> PA6/D12, swapping the two constants) or add a divider on the DT line.

## Build / flash / monitor

The same commands work on Linux, Windows, and macOS — PlatformIO, the STM32
Arduino core, and the firmware are all cross-platform. Nothing in
`platformio.ini` or `src/main.cpp` is OS-specific.

```bash
pio run                       # compile (pulls bogde/HX711 automatically)
pio run -t upload             # flash via onboard ST-Link
pio device monitor -b 115200  # view output (port auto-detected)
pio device list               # list serial ports if auto-detect misses
```

The only per-OS difference is the serial port name — `/dev/ttyACM0` on Linux,
`COMx` on Windows, `/dev/cu.usbmodem*` on macOS. `pio device monitor` and
`pio run -t upload` auto-detect it; pass `--port` / `--upload-port` only if
auto-detect picks the wrong one.

### Install PlatformIO

- **VS Code extension (all OSes, easiest):** install the "PlatformIO IDE"
  extension. Build/Upload/Monitor are the toolbar buttons; the bundled `pio` CLI
  is at `~/.platformio/penv/bin/pio` (Linux/macOS) or
  `%USERPROFILE%\.platformio\penv\Scripts\platformio.exe` (Windows).
- **CLI via installer script (needs Python 3):**
  ```bash
  python3 -c "$(curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py)"
  ```

### Windows setup

- **ST-Link driver:** install ST's ST-LINK USB driver (STSW-LINK009) so uploads
  work. Windows 10/11 often auto-installs it; if `pio run -t upload` can't find
  the probe, install it manually from st.com.
- **Serial access:** no extra permissions needed. Find the port with
  `pio device list` (e.g. `COM5`).
- **PATH (CLI users):** add
  `%USERPROFILE%\.platformio\penv\Scripts` to PATH, or use the "PlatformIO Core
  CLI" terminal from the VS Code extension.

### Linux setup

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
