/*
 * Adds support for the SparkFun Qwiic Keypad (12 button), which communicates over an I2C bus.
 * A firmware running on an ATtiny84 scans the key matrix, debounces it, and queues each key press (as an ASCII
 * character) in a FIFO that is read out over I2C. The device reports presses only: there are no release events.
 * Firmware: https://github.com/sparkfun/Qwiic_Keypad (accessed October 2026)
 * SparkFun's Arduino library: https://github.com/sparkfun/SparkFun_Qwiic_Keypad_Arduino_Library (accessed October
 * 2026)
 */

#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "registers.h"

namespace esphome::qwiic_keypad {

class QwiicKeypadComponent final : public Component, public i2c::I2CDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

 protected:
  uint8_t firmware_major_{0};
  uint8_t firmware_minor_{0};

  enum ErrorCode {
    NONE = 0,
    ERROR_COMMUNICATION_FAILED,
    ERROR_WRONG_CHIP_ID,
  } error_code_{NONE};

  // Pops the oldest press off the device's FIFO. `key` is 0 when the FIFO was empty.
  bool read_next_press_(uint8_t &key, uint16_t &time_since_pressed);
};

}  // namespace esphome::qwiic_keypad
