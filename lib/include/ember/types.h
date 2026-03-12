#ifndef EMBER_TYPES_H
#define EMBER_TYPES_H

#include <QString>

namespace Ember {

// Temperature unit enum
enum class TempUnit { Celsius = 0x00, Fahrenheit = 0x01 };

// Liquid state enum
enum class LiquidState {
  Unknown = 0x00,
  Empty = 0x01,
  Half = 0x02,
  Full = 0x03
};

// Mug event enum
enum class MugEvent {
  None = 0x00,
  CupRemoved = 0x01,
  CupPlaced = 0x02,
  TempReached = 0x03,
  BatteryLow = 0x04,
  HeatingStarted = 0x05,
  HeatingStopped = 0x06
};

// Battery state enum
enum class BatteryState {
  Unknown = 0x00,
  Charging = 0x01,
  Discharging = 0x02,
  Full = 0x03,
  NotCharging = 0x04
};

// Conversion utilities
inline float celsiusToFahrenheit(float celsius) {
  return celsius * 9.0f / 5.0f + 32.0f;
}

inline float fahrenheitToCelsius(float fahrenheit) {
  return (fahrenheit - 32.0f) * 5.0f / 9.0f;
}

inline QString tempUnitToString(TempUnit unit) {
  switch (unit) {
  case TempUnit::Celsius:
    return QStringLiteral("Celsius");
  case TempUnit::Fahrenheit:
    return QStringLiteral("Fahrenheit");
  default:
    return QStringLiteral("Unknown");
  }
}

inline QString liquidStateToString(LiquidState state) {
  switch (state) {
  case LiquidState::Empty:
    return QStringLiteral("Empty");
  case LiquidState::Half:
    return QStringLiteral("Half");
  case LiquidState::Full:
    return QStringLiteral("Full");
  default:
    return QStringLiteral("Unknown");
  }
}

inline QString batteryStateToString(BatteryState state) {
  switch (state) {
  case BatteryState::Charging:
    return QStringLiteral("Charging");
  case BatteryState::Discharging:
    return QStringLiteral("Discharging");
  case BatteryState::Full:
    return QStringLiteral("Full");
  case BatteryState::NotCharging:
    return QStringLiteral("Not Charging");
  default:
    return QStringLiteral("Unknown");
  }
}

inline QString mugEventToString(MugEvent event) {
  switch (event) {
  case MugEvent::CupRemoved:
    return QStringLiteral("Cup Removed");
  case MugEvent::CupPlaced:
    return QStringLiteral("Cup Placed");
  case MugEvent::TempReached:
    return QStringLiteral("Temperature Reached");
  case MugEvent::BatteryLow:
    return QStringLiteral("Battery Low");
  case MugEvent::HeatingStarted:
    return QStringLiteral("Heating Started");
  case MugEvent::HeatingStopped:
    return QStringLiteral("Heating Stopped");
  default:
    return QStringLiteral("None");
  }
}

} // namespace Ember

#endif // EMBER_TYPES_H
