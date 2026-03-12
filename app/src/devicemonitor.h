#ifndef DEVICEMONITOR_H
#define DEVICEMONITOR_H

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QList>
#include <QObject>
#include <QTimer>
#include <ember/mug.h>

class DeviceMonitor : public QObject {
  Q_OBJECT

public:
  explicit DeviceMonitor(QObject *parent = nullptr);
  ~DeviceMonitor();

  void startMonitoring();
  void stopMonitoring();

  QList<QBluetoothDeviceInfo> availableDevices() const;
  bool isScanning() const;

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
