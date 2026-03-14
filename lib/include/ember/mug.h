#ifndef EMBER_MUG_H
#define EMBER_MUG_H

#include "types.h"
#include <QBluetoothDeviceInfo>
#include <QBluetoothUuid>
#include <QLowEnergyController>
#include <QObject>

namespace Ember {

class Service;

class Mug : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged)
  Q_PROPERTY(float currentTemp READ currentTemp NOTIFY currentTempChanged)
  Q_PROPERTY(float targetTemp READ targetTemp NOTIFY targetTempChanged)
  Q_PROPERTY(TempUnit tempUnit READ tempUnit NOTIFY tempUnitChanged)
  Q_PROPERTY(int batteryLevel READ batteryLevel NOTIFY batteryLevelChanged)
  Q_PROPERTY(
      BatteryState batteryState READ batteryState NOTIFY batteryStateChanged)
  Q_PROPERTY(LiquidState liquidState READ liquidState NOTIFY liquidStateChanged)
  Q_PROPERTY(bool heating READ isHeating NOTIFY heatingChanged)
  Q_PROPERTY(QString name READ name NOTIFY nameChanged)

public:
  // Static device validation
  static bool isEmberMug(const QBluetoothDeviceInfo &device);
  static QBluetoothUuid serviceUuid();

  // Constructor - controller must already be connected
  explicit Mug(QLowEnergyController *controller, QObject *parent = nullptr);
  ~Mug();

  // Initialize service discovery (call after construction)
  void initialize();

  // State properties
  bool isReady() const;
  float currentTemp() const;
  float targetTemp() const;
  TempUnit tempUnit() const;
  int batteryLevel() const;
  BatteryState batteryState() const;
  LiquidState liquidState() const;
  bool isHeating() const;
  QString name() const;

  // Commands
  Q_INVOKABLE void setTargetTemperature(float celsius);
  Q_INVOKABLE void setTemperatureUnit(TempUnit unit);
  Q_INVOKABLE void refresh();

signals:
  void readyChanged();
  void error(const QString &message);
  void stateUpdated();
  void currentTempChanged();
  void targetTempChanged();
  void tempUnitChanged();
  void batteryLevelChanged();
  void batteryStateChanged();
  void liquidStateChanged();
  void heatingChanged();
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
