#ifndef EMBER_MUG_H
#define EMBER_MUG_H

/**
 * @file mug.h
 * @brief Ember Mug BLE device interface.
 */

#include "types.h"
#include <QBluetoothDeviceInfo>
#include <QBluetoothUuid>
#include <QLowEnergyController>
#include <QObject>

namespace Ember {

class Service;

/**
 * @brief Interface to an Ember Mug over Bluetooth LE.
 *
 * This class manages communication with an Ember Mug device. It handles
 * service discovery, characteristic subscriptions, and provides a Qt
 * property-based interface for monitoring and controlling the mug.
 *
 * Usage:
 * 1. Create a QLowEnergyController and connect to the device
 * 2. Construct a Mug instance with the connected controller
 * 3. Call initialize() to begin service discovery
 * 4. Wait for readyChanged() signal before accessing properties
 *
 * @note The controller must already be connected before constructing Mug.
 */
class Mug : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Mug)

  /** @brief True when the mug is ready for use. */
  Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged)

  /** @brief Current liquid temperature in Celsius. */
  Q_PROPERTY(float currentTemp READ currentTemp NOTIFY currentTempChanged)

  /** @brief Target temperature in Celsius. */
  Q_PROPERTY(float targetTemp READ targetTemp NOTIFY targetTempChanged)

  /** @brief Preferred temperature unit for display. */
  Q_PROPERTY(TempUnit tempUnit READ tempUnit NOTIFY tempUnitChanged)

  /** @brief Battery level as a percentage (0-100). */
  Q_PROPERTY(int batteryLevel READ batteryLevel NOTIFY batteryLevelChanged)

  /** @brief Current battery charging state. */
  Q_PROPERTY(
      BatteryState batteryState READ batteryState NOTIFY batteryStateChanged)

  /** @brief Current state of liquid in the mug. */
  Q_PROPERTY(LiquidState liquidState READ liquidState NOTIFY liquidStateChanged)

  /** @brief True if the mug is actively heating. */
  Q_PROPERTY(bool heating READ isHeating NOTIFY heatingChanged)

  /** @brief User-configured name of the mug. */
  Q_PROPERTY(QString name READ name NOTIFY nameChanged)

public:
  /**
   * @brief Check if a Bluetooth device is an Ember Mug.
   * @param device The device info to check.
   * @return True if the device is an Ember Mug.
   */
  static bool isEmberMug(const QBluetoothDeviceInfo &device);

  /**
   * @brief Get the Ember Mug BLE service UUID.
   * @return The service UUID used to identify Ember Mugs.
   */
  static QBluetoothUuid serviceUuid();

  /**
   * @brief Construct a Mug interface.
   * @param controller Connected BLE controller for the mug device.
   * @param parent Optional parent QObject.
   *
   * @note The controller must already be connected. Call initialize()
   *       after construction to begin service discovery.
   */
  explicit Mug(QLowEnergyController *controller, QObject *parent = nullptr);

  ~Mug() override;

  /**
   * @brief Begin service discovery and initialization.
   *
   * This starts asynchronous service discovery. When complete,
   * readyChanged() will be emitted with isReady() returning true.
   */
  void initialize();

  /**
   * @brief Check if the mug is ready for use.
   * @return True if service discovery is complete and properties are valid.
   */
  [[nodiscard]] bool isReady() const;

  /**
   * @brief Get the current liquid temperature.
   * @return Temperature in Celsius.
   */
  [[nodiscard]] float currentTemp() const;

  /**
   * @brief Get the target temperature.
   * @return Temperature in Celsius.
   */
  [[nodiscard]] float targetTemp() const;

  /**
   * @brief Get the preferred temperature unit.
   * @return The temperature unit setting.
   */
  [[nodiscard]] TempUnit tempUnit() const;

  /**
   * @brief Get the battery level.
   * @return Battery percentage (0-100).
   */
  [[nodiscard]] int batteryLevel() const;

  /**
   * @brief Get the battery charging state.
   * @return The current battery state.
   */
  [[nodiscard]] BatteryState batteryState() const;

  /**
   * @brief Get the liquid state.
   * @return The current liquid state.
   */
  [[nodiscard]] LiquidState liquidState() const;

  /**
   * @brief Check if the mug is actively heating.
   * @return True if heating is active.
   */
  [[nodiscard]] bool isHeating() const;

  /**
   * @brief Get the mug's configured name.
   * @return The mug name.
   */
  [[nodiscard]] QString name() const;

  /**
   * @brief Set the target temperature.
   * @param celsius Target temperature in Celsius.
   */
  Q_INVOKABLE void setTargetTemperature(float celsius);

  /**
   * @brief Set the preferred temperature unit.
   * @param unit The temperature unit to use.
   */
  Q_INVOKABLE void setTemperatureUnit(TempUnit unit);

  /**
   * @brief Request a refresh of all mug properties.
   *
   * Forces a re-read of all characteristics from the device.
   */
  Q_INVOKABLE void refresh();

signals:
  /** @brief Emitted when the ready state changes. */
  void readyChanged();

  /**
   * @brief Emitted when an error occurs.
   * @param message Description of the error.
   */
  void error(const QString &message);

  /** @brief Emitted when any mug state is updated. */
  void stateUpdated();

  /** @brief Emitted when the current temperature changes. */
  void currentTempChanged();

  /** @brief Emitted when the target temperature changes. */
  void targetTempChanged();

  /** @brief Emitted when the temperature unit changes. */
  void tempUnitChanged();

  /** @brief Emitted when the battery level changes. */
  void batteryLevelChanged();

  /** @brief Emitted when the battery state changes. */
  void batteryStateChanged();

  /** @brief Emitted when the liquid state changes. */
  void liquidStateChanged();

  /** @brief Emitted when the heating state changes. */
  void heatingChanged();

  /** @brief Emitted when the mug name changes. */
  void nameChanged();

private slots:
  void onServiceReady();
  void onServiceError(const QString &error);
  void onMugNameReceived(const QString &name);
  void onCurrentTempReceived(float tempCelsius);
  void onTargetTempReceived(float tempCelsius);
  void onTempUnitReceived(quint8 unit);
  void onBatteryReceived(int level, bool charging);
  void onLiquidStateReceived(quint8 state);
  void onPushEventReceived(quint8 event);

private:
  QLowEnergyController *m_controller;
  Service *m_service;

  bool m_ready = false;
  float m_currentTemp = 0.0f;
  float m_targetTemp = 0.0f;
  TempUnit m_tempUnit = TempUnit::Celsius;
  int m_batteryLevel = 0;
  BatteryState m_batteryState = BatteryState::Unknown;
  LiquidState m_liquidState = LiquidState::Unknown;
  bool m_isHeating = false;
  QString m_name;
};

} // namespace Ember

#endif // EMBER_MUG_H
