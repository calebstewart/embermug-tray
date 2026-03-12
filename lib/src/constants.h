#ifndef EMBER_CONSTANTS_H
#define EMBER_CONSTANTS_H

#include <QtGlobal>

namespace Ember {
namespace Constants {

// Service UUID for Ember Mug (v2)
constexpr const char *SERVICE_UUID = "fc543622-236c-4c94-8fa9-944a3e5353fa";

// Characteristic UUIDs
constexpr const char *CHARACTERISTIC_WRITE =
    "fc543623-236c-4c94-8fa9-944a3e5353fa";
constexpr const char *CHARACTERISTIC_NOTIFY =
    "fc543624-236c-4c94-8fa9-944a3e5353fa";

// Temperature range (Celsius)
constexpr float MIN_TEMPERATURE_C = 0.0f;
constexpr float MAX_TEMPERATURE_C = 62.0f;

// Temperature range (Fahrenheit)
constexpr float MIN_TEMPERATURE_F = 32.0f;
constexpr float MAX_TEMPERATURE_F = 143.6f;

// Command bytes
constexpr quint8 COMMAND_PING = 0x01;
constexpr quint8 COMMAND_GET_INFO = 0x02;
constexpr quint8 COMMAND_SET_TARGET_TEMP = 0x03;
constexpr quint8 COMMAND_SET_TEMP_UNIT = 0x04;
constexpr quint8 COMMAND_SET_MUG_NAME = 0x05;
constexpr quint8 COMMAND_SET_MUG_COLOR = 0x06;
constexpr quint8 COMMAND_GET_MUG_STATE = 0x07;
constexpr quint8 COMMAND_SET_HEATING = 0x08;
constexpr quint8 COMMAND_SET_CUP_POSITION = 0x09;

// Response bytes
constexpr quint8 RESPONSE_ACK = 0x01;
constexpr quint8 RESPONSE_NACK = 0x02;
constexpr quint8 RESPONSE_DATA = 0x03;

// Packet structure
constexpr size_t HEADER_SIZE = 3;   // Sync (2) + Command (1)
constexpr size_t CHECKSUM_SIZE = 1; // Single byte checksum
constexpr quint16 SYNC_BYTE_1 = 0xFD;
constexpr quint16 SYNC_BYTE_2 = 0x0C;

// Temperature unit
constexpr quint8 TEMP_UNIT_CELSIUS = 0x00;
constexpr quint8 TEMP_UNIT_FAHRENHEIT = 0x01;

// Battery level ranges
constexpr int BATTERY_MIN = 0;
constexpr int BATTERY_MAX = 100;

} // namespace Constants
} // namespace Ember

#endif // EMBER_CONSTANTS_H
