# esphome_qwiic_keypad

[![CI](https://github.com/ThatOneNeji/esphome_qwiic_keypad/actions/workflows/ci.yml/badge.svg)](https://github.com/ThatOneNeji/esphome_qwiic_keypad/actions/workflows/ci.yml)

An [ESPHome](https://esphome.io/) external component for the
[SparkFun Qwiic Keypad - 12 Button](https://www.sparkfun.com/sparkfun-qwiic-keypad-12-button.html): read key presses
over I2C, with per-key binary sensors and [`key_collector`](https://esphome.io/components/key_collector/) support. It
works like ESPHome's own [`matrix_keypad`](https://esphome.io/components/matrix_keypad/), without needing any GPIO
pins for the keys.

## Installation

Add this repository as a [git external component](https://esphome.io/components/external_components#git):

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/ThatOneNeji/esphome_qwiic_keypad
      ref: main
```

Minimum ESPHome version: `2026.7.0`.

## Quick start

```yaml
i2c:
  sda: GPIO21
  scl: GPIO22

qwiic_keypad:
  id: keypad
  on_key:
    - logger.log:
        format: "Key %c pressed"
        args: ["x"]

binary_sensor:
  - platform: qwiic_keypad
    qwiic_keypad_id: keypad
    name: "Keypad 5"
    key: "5"

key_collector:
  - id: pincode
    source_id: keypad
    min_length: 4
    max_length: 4
    end_keys: "#"
    end_key_required: true
    back_keys: "*"
    on_result:
      - logger.log:
          format: "PIN entered: %s"
          args: ["x.c_str()"]
```

See [`example.yaml`](example.yaml) / [`example.esp-idf.yaml`](example.esp-idf.yaml) for complete configs, and
**[components/qwiic_keypad/README.md](components/qwiic_keypad/README.md)** for the hardware overview, wiring,
configuration reference, known limitations and a troubleshooting guide.

## Components

| Component                                           | Description                                                                                                                        |
|-----------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------|
| [`qwiic_keypad`](components/qwiic_keypad/README.md) | I2C hub with `on_key` and `key_collector` support, plus a `binary_sensor` platform for single keys, for the SparkFun Qwiic Keypad. |

## Development

[pre-commit](https://pre-commit.com/) is used for linting/formatting (same tools as [ESPHome](https://github.com/esphome/esphome) itself: `ruff`, `flake8`, `clang-format`, `yamllint`, `pyupgrade`):

```sh
pip install pre-commit
pre-commit install       # optional: run automatically on every commit
pre-commit run --all-files
```

CI (`.github/workflows/ci.yml`) runs the same pre-commit checks, plus `esphome config`/`esphome compile` against
the example configs (via the local-source fixtures in `.github/ci/`) on every push to `main` and every pull request.

API documentation can be generated with [Doxygen](https://www.doxygen.nl/) (`doxygen Doxyfile`, output in `docs/`).

## License

[MIT](LICENSE)
