/*
 * Virtual I2C register map of the SparkFun Qwiic Keypad.
 * Firmware (ATtiny84): https://github.com/sparkfun/Qwiic_Keypad/blob/master/Firmware/Qwiic_KeyPad/Qwiic_KeyPad.ino
 * Arduino library: https://github.com/sparkfun/SparkFun_Qwiic_Keypad_Arduino_Library
 *
 * Unlike the Qwiic Button, the only multi-byte register (time since pressed) is laid out big-endian: the MSB lives
 * at the lower register address.
 */

#pragma once

#include <cstdint>

namespace esphome::qwiic_keypad {

enum QwiicKeypadRegister : uint8_t {
  QWIIC_KEYPAD_ID = 0x00,
  QWIIC_KEYPAD_FIRMWARE_MAJOR = 0x01,
  QWIIC_KEYPAD_FIRMWARE_MINOR = 0x02,
  QWIIC_KEYPAD_FIFO_BUTTON = 0x03,              // ASCII key of the oldest press, 0 when the FIFO is empty
  QWIIC_KEYPAD_FIFO_TIME_SINCE_PRESSED = 0x04,  // uint16_t, big-endian, milliseconds
  QWIIC_KEYPAD_UPDATE_FIFO = 0x06,
  QWIIC_KEYPAD_I2C_ADDRESS = 0x07,  // writes go to EEPROM; never written by this component
};

// The ID register always holds the firmware's *default* address, even when the board is re-addressed
static const uint8_t QWIIC_KEYPAD_DEVICE_ID = 0x4B;
static const uint8_t QWIIC_KEYPAD_DEFAULT_I2C_ADDRESS = 0x4B;

// Writing this to QWIIC_KEYPAD_UPDATE_FIFO makes the device load the oldest press into the FIFO registers on the
// next read request
static const uint8_t QWIIC_KEYPAD_UPDATE_FIFO_COMMAND = 0x01;

// Depth of the device's press buffer; on overflow it drops the oldest press
static const uint8_t QWIIC_KEYPAD_FIFO_SIZE = 15;

// The characters the firmware reports, in physical position order (row by row, left to right). A key's index in
// this string is its position: row = position / QWIIC_KEYPAD_COLUMNS, col = position % QWIIC_KEYPAD_COLUMNS.
static const char *const QWIIC_KEYPAD_LAYOUT = "123456789*0#";
static const uint8_t QWIIC_KEYPAD_KEY_COUNT = 12;
static const uint8_t QWIIC_KEYPAD_COLUMNS = 3;

}  // namespace esphome::qwiic_keypad
