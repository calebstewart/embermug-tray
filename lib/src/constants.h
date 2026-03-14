#ifndef EMBER_CONSTANTS_H
#define EMBER_CONSTANTS_H

#include <QtGlobal>

namespace Ember::Constants {

// Service UUID for Ember Mug
constexpr const char *SERVICE_UUID = "fc543622-236c-4c94-8fa9-944a3e5353fa";

// Characteristic UUIDs (individual characteristics for each feature)
constexpr const char *CHAR_MUG_NAME = "fc540001-236c-4c94-8fa9-944a3e5353fa";
constexpr const char *CHAR_CURRENT_TEMP = "fc540002-236c-4c94-8fa9-944a3e5353fa";
constexpr const char *CHAR_TARGET_TEMP = "fc540003-236c-4c94-8fa9-944a3e5353fa";
constexpr const char *CHAR_TEMP_UNIT = "fc540004-236c-4c94-8fa9-944a3e5353fa";
constexpr const char *CHAR_BATTERY = "fc540007-236c-4c94-8fa9-944a3e5353fa";
constexpr const char *CHAR_LIQUID_STATE = "fc540008-236c-4c94-8fa9-944a3e5353fa";
constexpr const char *CHAR_PUSH_EVENTS = "fc540012-236c-4c94-8fa9-944a3e5353fa";
constexpr const char *CHAR_MUG_COLOR = "fc540014-236c-4c94-8fa9-944a3e5353fa";

// Temperature range (Celsius)
constexpr float MIN_TEMPERATURE_C = 50.0f;
constexpr float MAX_TEMPERATURE_C = 62.5f;

// Temperature range (Fahrenheit)
constexpr float MIN_TEMPERATURE_F = 120.0f;
constexpr float MAX_TEMPERATURE_F = 145.0f;

// Push event types (notifications from mug)
constexpr quint8 EVENT_REFRESH_BATTERY = 0x01;
constexpr quint8 EVENT_CHARGING = 0x02;
constexpr quint8 EVENT_NOT_CHARGING = 0x03;
constexpr quint8 EVENT_REFRESH_TARGET_TEMP = 0x04;
constexpr quint8 EVENT_REFRESH_CURRENT_TEMP = 0x05;
constexpr quint8 EVENT_REFRESH_LIQUID_LEVEL = 0x07;
constexpr quint8 EVENT_REFRESH_LIQUID_STATE = 0x08;

// Battery level ranges
constexpr int BATTERY_MIN = 5;
constexpr int BATTERY_MAX = 100;

} // namespace Ember::Constants

#endif // EMBER_CONSTANTS_H
