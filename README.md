# esphome_qwiic_keypad

[![CI](https://github.com/ThatOneNeji/esphome_qwiic_keypad/actions/workflows/ci.yml/badge.svg)](https://github.com/ThatOneNeji/esphome_qwiic_keypad/actions/workflows/ci.yml)

An [ESPHome](https://esphome.io/) external component for the
[SparkFun Qwiic Keypad - 12 Button](https://www.sparkfun.com/sparkfun-qwiic-keypad-12-button.html): read key presses
over I2C.

> **Work in progress.** The component currently logs key presses only. `on_key`, `key_collector` support and
> per-key `binary_sensor`s are planned.

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
  address: 0x4B
```

See [`example.yaml`](example.yaml) / [`example.esp-idf.yaml`](example.esp-idf.yaml) for complete configs.

## Development

[pre-commit](https://pre-commit.com/) is used for linting/formatting (same tools as [ESPHome](https://github.com/esphome/esphome) itself: `ruff`, `flake8`, `clang-format`, `yamllint`, `pyupgrade`):

```sh
pip install pre-commit
pre-commit install       # optional: run automatically on every commit
pre-commit run --all-files
```

CI (`.github/workflows/ci.yml`) runs the same pre-commit checks, plus `esphome config`/`esphome compile` against
the example configs (via the local-source fixtures in `.github/ci/`) on every push and pull request.

## License

[MIT](LICENSE)
