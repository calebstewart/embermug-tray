#ifndef EMBER_SERVICE_H
#define EMBER_SERVICE_H

#include <QBluetoothUuid>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QObject>

namespace Ember {

class Service : public QObject {
  Q_OBJECT

public:
  explicit Service(QLowEnergyController *controller, QObject *parent = nullptr);
  ~Service();

  // Initialize service discovery
  void initialize();

  // Service state
  bool isReady() const;

  // Operations
  bool writeCommand(const QByteArray &data);

signals:
  void ready();
  void error(const QString &message);
  void dataReceived(const QByteArray &data);

private slots:
  void onServiceDiscovered(const QBluetoothUuid &uuid);
  void onServiceDiscoveryFinished();
  void onServiceStateChanged(QLowEnergyService::ServiceState newState);
  void onCharacteristicChanged(const QLowEnergyCharacteristic &charInfo,
                               const QByteArray &value);
  void onServiceError(QLowEnergyService::ServiceError error);

private:
  void setupNotifications();

  QLowEnergyController *m_controller;
  QLowEnergyService *m_service = nullptr;
  bool m_ready = false;
};

} // namespace Ember

#endif // EMBER_SERVICE_H
