#ifndef EMBER_TYPES_H
#define EMBER_TYPES_H

/**
 * @file types.h
 * @brief Common types and utilities for Ember Mug communication.
 */

#include <QString>

namespace Ember {

/**
 * @brief Temperature unit for display and input.
 */
enum class TempUnit {
  Celsius = 0x00,    ///< Celsius temperature scale
  Fahrenheit = 0x01  ///< Fahrenheit temperature scale
};

/**
 * @brief State of the liquid in the mug.
 *
 * Values correspond to Ember Mug BLE protocol values.
 */
enum class LiquidState {
  Unknown = 0x00,  ///< State not yet determined
  Empty = 0x01,    ///< Mug is empty
  Filling = 0x02,  ///< Liquid is being poured
  Cold = 0x03,     ///< Liquid is below target temperature
  Cooling = 0x04,  ///< Liquid is cooling down toward target
  Heating = 0x05,  ///< Liquid is being heated toward target
  AtTarget = 0x06  ///< Liquid is at target temperature
};

/**
 * @brief Events pushed from the mug.
 */
enum class MugEvent {
  None = 0x00,           ///< No event
  CupRemoved = 0x01,     ///< Mug removed from charging coaster
  CupPlaced = 0x02,      ///< Mug placed on charging coaster
  TempReached = 0x03,    ///< Target temperature reached
  BatteryLow = 0x04,     ///< Battery level is low
  HeatingStarted = 0x05, ///< Mug started heating
  HeatingStopped = 0x06  ///< Mug stopped heating
};

/**
 * @brief Battery charging state.
 */
enum class BatteryState {
  Unknown = 0x00,      ///< State not yet determined
  Charging = 0x01,     ///< Battery is charging
  Discharging = 0x02,  ///< Battery is discharging (in use)
  Full = 0x03,         ///< Battery is fully charged
  NotCharging = 0x04   ///< On coaster but not charging
};

/**
 * @brief Convert Celsius to Fahrenheit.
 * @param celsius Temperature in Celsius.
 * @return Temperature in Fahrenheit.
 */
inline float celsiusToFahrenheit(float celsius) {
  return celsius * 9.0f / 5.0f + 32.0f;
}

/**
 * @brief Convert Fahrenheit to Celsius.
 * @param fahrenheit Temperature in Fahrenheit.
 * @return Temperature in Celsius.
 */
inline float fahrenheitToCelsius(float fahrenheit) {
  return (fahrenheit - 32.0f) * 5.0f / 9.0f;
}

/**
 * @brief Get human-readable string for a temperature unit.
 * @param unit The temperature unit.
 * @return Display string for the unit.
 */
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

/**
 * @brief Get human-readable string for a liquid state.
 * @param state The liquid state.
 * @return Display string for the state.
 */
inline QString liquidStateToString(LiquidState state) {
  switch (state) {
  case LiquidState::Empty:
    return QStringLiteral("Empty");
  case LiquidState::Filling:
    return QStringLiteral("Filling");
  case LiquidState::Cold:
    return QStringLiteral("Cold");
  case LiquidState::Cooling:
    return QStringLiteral("Cooling");
  case LiquidState::Heating:
    return QStringLiteral("Heating");
  case LiquidState::AtTarget:
    return QStringLiteral("At Target");
  default:
    return QStringLiteral("Unknown");
  }
}

/**
 * @brief Get human-readable string for a battery state.
 * @param state The battery state.
 * @return Display string for the state.
 */
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

/**
 * @brief Get human-readable string for a mug event.
 * @param event The mug event.
 * @return Display string for the event.
 */
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
