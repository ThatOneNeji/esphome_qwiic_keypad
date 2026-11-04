#include "qwiic_keypad.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome::qwiic_keypad {

static const char *const TAG = "qwiic_keypad";

void QwiicKeypadComponent::setup() {
  // Verify I2C communication by reading and verifying the chip ID
  uint8_t chip_id;
  if (!this->read_byte(QWIIC_KEYPAD_ID, &chip_id)) {
    ESP_LOGE(TAG, "Failed to read chip ID");
    this->error_code_ = ERROR_COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }

  if (chip_id != QWIIC_KEYPAD_DEVICE_ID) {
    ESP_LOGE(TAG, "Unknown chip ID 0x%02X", chip_id);
    this->error_code_ = ERROR_WRONG_CHIP_ID;
    this->mark_failed();
    return;
  }

  uint8_t version[2];
  if (!this->read_bytes(QWIIC_KEYPAD_FIRMWARE_MAJOR, version, 2)) {
    ESP_LOGE(TAG, "Failed to read firmware version");
    this->error_code_ = ERROR_COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }
  this->firmware_major_ = version[0];
  this->firmware_minor_ = version[1];

  // Discard presses queued before boot, so they aren't replayed as if they just happened
  uint8_t discarded = 0;
  for (uint8_t i = 0; i < QWIIC_KEYPAD_FIFO_SIZE; i++) {
    uint8_t key;
    uint16_t time_since_pressed;
    if (!this->read_next_press_(key, time_since_pressed) || key == 0)
      break;
    discarded++;
  }
  if (discarded > 0)
    ESP_LOGD(TAG, "Discarded %u key press(es) queued before boot", discarded);
}

void QwiicKeypadComponent::loop() {
  // Bounded by the FIFO depth, so a misbehaving device can't keep us here forever
  for (uint8_t i = 0; i < QWIIC_KEYPAD_FIFO_SIZE; i++) {
    uint8_t key;
    uint16_t time_since_pressed;
    if (!this->read_next_press_(key, time_since_pressed)) {
      ESP_LOGW(TAG, ESP_LOG_MSG_COMM_FAIL);
      return;
    }
    if (key == 0)
      return;

    ESP_LOGD(TAG, "Key '%c' pressed (%u ms ago)", key, time_since_pressed);
  }
}

bool QwiicKeypadComponent::read_next_press_(uint8_t &key, uint16_t &time_since_pressed) {
  // The device only loads the next press when the read request arrives, so the command has to be written first
  if (!this->write_byte(QWIIC_KEYPAD_UPDATE_FIFO, QWIIC_KEYPAD_UPDATE_FIFO_COMMAND))
    return false;

  // Button, then time since pressed (MSB, LSB), read in one go
  uint8_t data[3];
  if (!this->read_bytes(QWIIC_KEYPAD_FIFO_BUTTON, data, 3))
    return false;

  key = data[0];
  time_since_pressed = encode_uint16(data[1], data[2]);
  return true;
}

void QwiicKeypadComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Qwiic Keypad:");
  ESP_LOGCONFIG(TAG, "  Firmware Version: %u.%u", this->firmware_major_, this->firmware_minor_);

  switch (this->error_code_) {
    case NONE:
      break;
    case ERROR_COMMUNICATION_FAILED:
      ESP_LOGE(TAG, ESP_LOG_MSG_COMM_FAIL);
      break;
    case ERROR_WRONG_CHIP_ID:
      ESP_LOGE(TAG, "Unknown chip ID");
      break;
  }

  LOG_I2C_DEVICE(this);
}

}  // namespace esphome::qwiic_keypad
