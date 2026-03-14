#ifndef EMBER_SERVICE_H
#define EMBER_SERVICE_H

#include <QBluetoothUuid>
#include <QLowEnergyCharacteristic>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QObject>

namespace Ember {

class Service : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Service)

public:
  explicit Service(QLowEnergyController *controller, QObject *parent = nullptr);
  ~Service() override;

  void initialize();
  [[nodiscard]] bool isReady() const;

  // Read operations
  void readMugName();
  void readCurrentTemp();
  void readTargetTemp();
  void readTempUnit();
  void readBattery();
  void readLiquidState();

  // Write operations
  void writeTargetTemp(quint16 tempCelsiusTimes100);
  void writeTempUnit(quint8 unit);

signals:
  void ready();
  void error(const QString &message);

  // Data received signals
  void mugNameReceived(const QString &name);
  void currentTempReceived(float tempCelsius);
  void targetTempReceived(float tempCelsius);
  void tempUnitReceived(quint8 unit);
  void batteryReceived(int level, bool charging);
  void liquidStateReceived(quint8 state);
  void pushEventReceived(quint8 event);

private slots:
  void onServiceDiscovered(const QBluetoothUuid &uuid);
  void onServiceDiscoveryFinished();
  void onServiceStateChanged(QLowEnergyService::ServiceState newState);
  void onCharacteristicRead(const QLowEnergyCharacteristic &charInfo,
                            const QByteArray &value);
  void onCharacteristicChanged(const QLowEnergyCharacteristic &charInfo,
                               const QByteArray &value);
  void onServiceError(QLowEnergyService::ServiceError error);

private:
  void setupNotifications();
  void readAllCharacteristics();
  QLowEnergyCharacteristic findCharacteristic(const char *uuid);

  QLowEnergyController *m_controller;
  QLowEnergyService *m_service = nullptr;
  bool m_ready = false;
};

} // namespace Ember

#endif // EMBER_SERVICE_H
