# CLAUDE.md

ESPHome external component for the SparkFun Qwiic Keypad (12 button). The full roadmap is in [tmp/roadmap.md](tmp/roadmap.md), ready for Saturday 2026-10-31. It's based on the qwiic_btn code (`/home/neji/repos/_mine/esphome_qwiic_btn`), the keypad's firmware source and SparkFun's library, and ESPHome's `matrix_keypad`, `key_collector` and `key_provider` source.

## What's different from what we expected

- **The register map is different.** Both boards use the same chip family, but the firmware isn't the same, so qwiic_btn's `registers.h` can't be reused. The component's structure can be reused (setup/loop, error codes, Python schema).
  - The keypad has 8 registers and its default address is `0x4B`.
  - Reading works like this: write `0x01` to register `0x06`, then read 3 bytes from `0x03` (the key and how long ago it was pressed). Repeat until the key byte is `0`.
- **It only reports key presses.** There's no release event and no "key is held" state, and debouncing happens on the keypad itself. `matrix_keypad`'s binary sensors stay ON while a key is held, so ours generate the release themselves (see the decision below).
- **The keypad sends plain characters** (`'0'-'9'`, `*`, `#`), so plugging it into `key_collector` is almost free. We make the class a `KeyProvider` and call `send_key_()` for each press.
- **`Doxyfile` is ESPHome's own file**, not ours (`PROJECT_NAME = ESPHome`, `INPUT = esphome`). The copy in qwiic_btn is the same, so it was never adapted there either. The roadmap lists the settings to change.

## Confirmed on hardware (2026-11-04, ESP-IDF, I2C at 800 kHz)

- Chip ID and firmware `1.0` read correctly. All 12 keys report the right character, picked up 0–17 ms after the press. The write-`0x06`-then-read sequence works, with no communication errors over several minutes of polling.
- Holding a key reports **one** press: no auto-repeat, and nothing on release. This confirms the generated-release design.
- **Known limitation (firmware, can't fix from our side): overlapping presses are lost.** The firmware calls the Arduino `Keypad` library's `getKey()`, which only reports slot 0 of its key list. A key that goes down while the previous key is still held, or within about 20 ms of its release, lands in slot 1 and never reaches the FIFO. Seen in a fast-typing test: `1`, `3`, `4`… with the `2` missing. Document this in the README.
- **Phase 2 confirmed (same day):** `on_key`, `key_collector` progress/result/timeout and `*` as backspace all work. The generated release fires about 100 ms after each press, and a repeat press inside the delay produces release-then-press as designed.
- **Known issue (hardware): stiff new keys can register one tap as two presses.** The `*` key did this three times (gaps of 86–112 ms), so one tap gave two backspaces in `key_collector`. The firmware's `Keypad` library only debounces for 10 ms, so a slower bounce on a stiff membrane key reaches the FIFO as a second press. Document this in the README, together with the lost-overlapping-press limitation. A possible future mitigation (not built, not decided): an optional filter that ignores a repeat of the same key within N ms, using the device's time-since-pressed values.

## Hardware (from the photos in `images/`)

- Keypad board **v1.0**, MCU **ATtiny84** (`TINY84-20U`), which matches the SparkFun firmware the roadmap is based on. Key layout: `1 2 3 / 4 5 6 / 7 8 9 / * 0 #`.
- **ADR jumper open**, so the address is the default `0x4B`.
- **I2C pull-up jumper closed**, so the keypad's 2.2 kΩ pull-ups are on.
- **INT is not wired.** It's only a solder pad, not part of the Qwiic connector, and the keypad is connected by a Qwiic cable. Phase 1 polls the bus; the INT pin stays an optional extra.
- Test rig: ESP32-WROOM-32 dev board. Devices on the I2C bus:

  | Device                     | Address                       |
  |----------------------------|-------------------------------|
  | Qwiic Keypad               | `0x4B`                        |
  | Qwiic Button               | `0x6F`                        |
  | MCP23017 (Waveshare board) | `0x20`-`0x27`, set by A0-A2   |

  `i2c: scan: true` will confirm the MCP23017's address. No address clashes. Up to three sets of pull-ups are in parallel, so start at 100 kHz. If there are errors at 400 kHz, try cutting the keypad's I2C jumper first.
- The 2-channel relay board is driven by GPIO, not I2C, and is not part of this project.
- **Testing workflow:** the user builds and flashes from their own ESPHome instance, which holds the secrets and the shared `common/` package files. Hardware test configs (e.g. `tmp/board09.yaml`) therefore pull the component from GitHub (`ref: develop`), so changes must be pushed before they can be tested. Test configs cover the keypad only, not qwiic_btn, since users won't necessarily have both.
- `images/` holds README-sized copies (≤1200 px, metadata stripped). The full-size originals are in `images/originals/`, which git ignores.

## Order for Saturday

1. Tidy the repo: commit the dot-files, fix the Doxyfile, set up pre-commit, copy CI from qwiic_btn.
2. Get the keypad talking on the ESP32 so presses show up in the log. Start the I2C bus at 100 kHz, not the 800 kHz from the qwiic_btn notes.
3. Add `on_key` and the `key_provider` hook.
4. Test with `key_collector` (PIN entry with `#` as enter and `*` as backspace).
5. Add the per-key `binary_sensor`.
6. Extras if there's time: the keypad's INT pin, slower polling, README and CI.

## Decision: follow `matrix_keypad` as closely as possible (decided 2026-11-04)

Mirror ESPHome's [`matrix_keypad`](https://github.com/esphome/esphome/tree/dev/esphome/components/matrix_keypad) one-to-one wherever the hardware allows:

- The same listener interface: `button_pressed/released(row, col)` and `key_pressed/released(key)`.
- The same `on_key:` trigger (`x` = key code), and `KeyProvider` + `send_key_()` for `key_collector`.
- The same `binary_sensor` schema: `key:` **or** `row:`/`col:`, with matrix_keypad's `check_button()` validation and `BinarySensorInitiallyOff`.
- The same optional `keys:` string (one key code per position, validated like `check_keys`), defaulting to the printed labels `123456789*0#`. We need the character → position table for `row:`/`col:` anyway.
- Only one key is active at a time: a new key releases the previous one first.
- Left out because the ATtiny scans the matrix: `rows`, `columns`, `has_diodes`, `has_pulldowns` and `debounce_time`.

The one unavoidable difference: the release is **generated after an optional `release_delay:`** (default about 100 ms) instead of coming from the hardware. A repeat press of the same key inside the delay publishes OFF then ON, so every press still fires `on_press`. No `LATCH` mode, since matrix_keypad has none.

## Open questions

- Minimum ESPHome version: keep `2026.7.0` if the first CI run passes on it.
