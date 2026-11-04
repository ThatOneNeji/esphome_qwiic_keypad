#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/qwiic_keypad/qwiic_keypad.h"

namespace esphome::qwiic_keypad {

// Mirrors matrix_keypad's MatrixKeypadBinarySensor: ON while the key counts as pressed. The keypad never reports a
// release, so "pressed" lasts until the hub generates one (after `release_delay`, or on the next key press).
class QwiicKeypadBinarySensor final : public QwiicKeypadListener, public binary_sensor::BinarySensorInitiallyOff {
 public:
  QwiicKeypadBinarySensor(uint8_t key) : has_key_(true), key_(key){};
  QwiicKeypadBinarySensor(const char *key) : has_key_(true), key_((uint8_t) key[0]){};
  QwiicKeypadBinarySensor(int row, int col) : has_key_(false), row_(row), col_(col){};

  void key_pressed(uint8_t key) override {
    if (!this->has_key_)
      return;
    if (key == this->key_)
      this->publish_state(true);
  }

  void key_released(uint8_t key) override {
    if (!this->has_key_)
      return;
    if (key == this->key_)
      this->publish_state(false);
  }

  void button_pressed(int row, int col) override {
    if (this->has_key_)
      return;
    if ((row == this->row_) && (col == this->col_))
      this->publish_state(true);
  }

  void button_released(int row, int col) override {
    if (this->has_key_)
      return;
    if ((row == this->row_) && (col == this->col_))
      this->publish_state(false);
  }

 protected:
  bool has_key_;
  uint8_t key_;
  int row_;
  int col_;
};

}  // namespace esphome::qwiic_keypad
