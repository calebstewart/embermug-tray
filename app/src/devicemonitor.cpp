#include "devicemonitor.h"
#include <QDebug>

DeviceMonitor::DeviceMonitor(QObject *parent)
    : QObject(parent),
      m_discoveryAgent(new QBluetoothDeviceDiscoveryAgent(this)) {
  m_discoveryAgent->setLowEnergyDiscoveryTimeout(10000);

  connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
          this, &DeviceMonitor::onDeviceDiscovered);
  connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this,
          &DeviceMonitor::onDiscoveryFinished);
  connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
          this, &DeviceMonitor::onDiscoveryError);

  // Set up periodic scan timer (30 seconds)
  m_scanTimer.setInterval(30000);
  connect(&m_scanTimer, &QTimer::timeout, this, &DeviceMonitor::performScan);
}

DeviceMonitor::~DeviceMonitor() { stopMonitoring(); }

void DeviceMonitor::startMonitoring() {
  if (m_monitoring)
    return;

  m_monitoring = true;
  performScan();
  m_scanTimer.start();
}

void DeviceMonitor::stopMonitoring() {
  m_monitoring = false;
  m_scanTimer.stop();
  if (m_discoveryAgent->isActive()) {
    m_discoveryAgent->stop();
  }
}

QList<QBluetoothDeviceInfo> DeviceMonitor::availableDevices() const {
  return m_availableDevices;
}

bool DeviceMonitor::isScanning() const { return m_discoveryAgent->isActive(); }

std::optional<QBluetoothDeviceInfo> DeviceMonitor::findByAddress(const QBluetoothAddress &addr) const {
  for (const auto &device : m_availableDevices) {
    if (device.address() == addr) {
      return device;
    }
  }
  return std::nullopt;
}

std::optional<QBluetoothDeviceInfo> DeviceMonitor::findByName(const QString &name) const {
  for (const auto &device : m_availableDevices) {
    if (device.name() == name) {
      return device;
    }
  }
  return std::nullopt;
}

void DeviceMonitor::performScan() {
  if (m_discoveryAgent->isActive()) {
    return;
  }

  m_availableDevices.clear();
  emit scanStarted();

  m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

void DeviceMonitor::onDeviceDiscovered(const QBluetoothDeviceInfo &device) {
  // Check if it's an Ember Mug
  if (!Ember::Mug::isEmberMug(device)) {
    return;
  }

  // Check if we already have this device
  for (const auto &existing : m_availableDevices) {
    if (existing.address() == device.address()) {
      return;
    }
  }

  qInfo() << "Found Ember Mug:" << device.name() << device.address().toString();
  m_availableDevices.append(device);
  emit devicesChanged();
}

void DeviceMonitor::onDiscoveryFinished() {
  qInfo() << "Device scan finished. Found" << m_availableDevices.size()
          << "Ember Mugs";
  emit scanFinished();
}

void DeviceMonitor::onDiscoveryError(
    QBluetoothDeviceDiscoveryAgent::Error error) {
  qWarning() << "Device discovery error:" << error;
  emit scanFinished();
}
