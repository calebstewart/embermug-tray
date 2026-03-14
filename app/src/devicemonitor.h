#ifndef DEVICEMONITOR_H
#define DEVICEMONITOR_H

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QList>
#include <QObject>
#include <QTimer>
#include <ember/mug.h>
#include <optional>

class DeviceMonitor : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DeviceMonitor)

public:
  explicit DeviceMonitor(QObject *parent = nullptr);
  ~DeviceMonitor() override;

  void startMonitoring();
  void stopMonitoring();

  [[nodiscard]] QList<QBluetoothDeviceInfo> availableDevices() const;
  [[nodiscard]] bool isScanning() const;

  [[nodiscard]] std::optional<QBluetoothDeviceInfo> findByAddress(const QBluetoothAddress &addr) const;
  [[nodiscard]] std::optional<QBluetoothDeviceInfo> findByName(const QString &name) const;

signals:
  void devicesChanged();
  void scanStarted();
  void scanFinished();

private slots:
  void onDeviceDiscovered(const QBluetoothDeviceInfo &device);
  void onDiscoveryFinished();
  void onDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error);
  void performScan();

private:
  QBluetoothDeviceDiscoveryAgent *m_discoveryAgent;
  QTimer m_scanTimer;
  QList<QBluetoothDeviceInfo> m_availableDevices;
  bool m_monitoring = false;
};

#endif // DEVICEMONITOR_H
