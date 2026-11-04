/*
 * Adds support for the SparkFun Qwiic Keypad (12 button), which communicates over an I2C bus.
 * A firmware running on an ATtiny84 scans the key matrix, debounces it, and queues each key press (as an ASCII
 * character) in a FIFO that is read out over I2C. The device reports presses only: there are no release events.
 * Firmware: https://github.com/sparkfun/Qwiic_Keypad (accessed October 2026)
 * SparkFun's Arduino library: https://github.com/sparkfun/SparkFun_Qwiic_Keypad_Arduino_Library (accessed October
 * 2026)
 *
 * Mirrors ESPHome's matrix_keypad component (listeners, on_key trigger, key_provider, `keys` mapping) as closely as
 * the hardware allows. Since the device never reports a release, a key is released `release_delay` after its press,
 * or as soon as another key is pressed.
 */

#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/key_provider/key_provider.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "registers.h"
#include <vector>

namespace esphome::qwiic_keypad {

class QwiicKeypadListener {
 public:
  virtual void button_pressed(int row, int col){};
  virtual void button_released(int row, int col){};
  virtual void key_pressed(uint8_t key){};
  virtual void key_released(uint8_t key){};
};

class QwiicKeypadKeyTrigger final : public Trigger<uint8_t> {};

class QwiicKeypadComponent final : public key_provider::KeyProvider, public Component, public i2c::I2CDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  /// `keys` holds one key code per position (see QWIIC_KEYPAD_LAYOUT); codegen passes a string literal.
  void set_keys(const char *keys) { this->keys_ = keys; }
  void set_release_delay(uint32_t release_delay) { this->release_delay_ = release_delay; }

  void register_listener(QwiicKeypadListener *listener);
  void register_key_trigger(QwiicKeypadKeyTrigger *trig);

 protected:
  const char *keys_{QWIIC_KEYPAD_LAYOUT};
  uint32_t release_delay_{100};
  uint8_t firmware_major_{0};
  uint8_t firmware_minor_{0};
  int pressed_key_{-1};  // position of the key currently held, or -1

  std::vector<QwiicKeypadListener *> listeners_{};
  std::vector<QwiicKeypadKeyTrigger *> key_triggers_;

  enum ErrorCode {
    NONE = 0,
    ERROR_COMMUNICATION_FAILED,
    ERROR_WRONG_CHIP_ID,
  } error_code_{NONE};

  // Pops the oldest press off the device's FIFO. `key` is 0 when the FIFO was empty.
  bool read_next_press_(uint8_t &key, uint16_t &time_since_pressed);
  void press_key_(int position);
  void release_key_();
};

}  // namespace esphome::qwiic_keypad
