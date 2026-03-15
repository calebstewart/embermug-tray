#include "connectionmanager.h"
#include <QBluetoothAddress>
#include <QDebug>
#include <QSettings>

ConnectionManager::ConnectionManager(QObject *parent) : QObject(parent) {
  m_initTimer.setSingleShot(true);
  connect(&m_initTimer, &QTimer::timeout, this,
          &ConnectionManager::onInitTimeout);
  connect(&m_localDevice, &QBluetoothLocalDevice::pairingFinished, this,
          &ConnectionManager::onPairingFinished);
}

ConnectionManager::~ConnectionManager() { cleanup(); }

void ConnectionManager::connectToDevice(const QBluetoothDeviceInfo &device) {
  if (m_controller != nullptr || m_connecting) {
    qWarning() << "Already connected or connecting";
    return;
  }

  m_connecting = true;
  m_currentDevice = device;
  emit connecting();

  // Ensure device is paired for write permissions on encrypted characteristics
  auto pairingStatus = m_localDevice.pairingStatus(device.address());
  if (pairingStatus == QBluetoothLocalDevice::Unpaired) {
    qInfo() << "Device not paired, requesting pairing:" << device.name()
            << device.address().toString();
    m_localDevice.requestPairing(device.address(),
                                 QBluetoothLocalDevice::Paired);
    return;
  }

  proceedWithConnection();
}

void ConnectionManager::proceedWithConnection() {
  m_controller = QLowEnergyController::createCentral(m_currentDevice, this);
  if (m_controller == nullptr) {
    m_connecting = false;
    emit connectionFailed(QStringLiteral("Failed to create BLE controller"));
    return;
  }

  // BLE devices typically use random addresses
  m_controller->setRemoteAddressType(QLowEnergyController::RandomAddress);

  connect(m_controller, &QLowEnergyController::connected, this,
          &ConnectionManager::onControllerConnected);
  connect(m_controller, &QLowEnergyController::disconnected, this,
          &ConnectionManager::onControllerDisconnected);
  connect(m_controller, &QLowEnergyController::errorOccurred, this,
          &ConnectionManager::onControllerError);

  qInfo() << "Connecting to device:" << m_currentDevice.name()
          << m_currentDevice.address().toString();
  m_controller->connectToDevice();
}

void ConnectionManager::disconnect() {
  if (m_controller != nullptr) {
    m_controller->disconnectFromDevice();
  }
  cleanup();
}

bool ConnectionManager::isConnected() const {
  return m_controller != nullptr &&
         m_controller->state() == QLowEnergyController::ConnectedState &&
         m_mug != nullptr && m_mug->isReady();
}

bool ConnectionManager::isConnecting() const { return m_connecting; }

Ember::Mug *ConnectionManager::mug() const { return m_mug; }

QBluetoothDeviceInfo ConnectionManager::connectedDevice() const {
  return m_currentDevice;
}

void ConnectionManager::onControllerConnected() {
  qInfo() << "BLE controller connected";
  emit connected();

  // Create Mug instance and initialize
  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory) - Qt parent ownership
  m_mug = new Ember::Mug(m_controller, this);
  connect(m_mug, &Ember::Mug::readyChanged, this,
          &ConnectionManager::onMugReady);
  connect(m_mug, &Ember::Mug::error, this, &ConnectionManager::onMugError);

  m_mug->initialize();
  m_initTimer.start(INIT_TIMEOUT_MS);
}

void ConnectionManager::onControllerDisconnected() {
  qInfo() << "BLE controller disconnected";
  m_connecting = false;
  cleanup();
  emit disconnected();
}

void ConnectionManager::onControllerError(QLowEnergyController::Error error) {
  QString errorMsg;
  switch (error) {
  case QLowEnergyController::NoError:
    return;
  case QLowEnergyController::UnknownError:
    errorMsg = QStringLiteral("Unknown error");
    break;
  case QLowEnergyController::UnknownRemoteDeviceError:
    errorMsg = QStringLiteral("Unknown remote device");
    break;
  case QLowEnergyController::NetworkError:
    errorMsg = QStringLiteral("Network error");
    break;
  case QLowEnergyController::InvalidBluetoothAdapterError:
    errorMsg = QStringLiteral("Invalid Bluetooth adapter");
    break;
  case QLowEnergyController::ConnectionError:
    errorMsg = QStringLiteral("Connection error");
    break;
  case QLowEnergyController::AdvertisingError:
    errorMsg = QStringLiteral("Advertising error");
    break;
  case QLowEnergyController::RemoteHostClosedError:
    errorMsg = QStringLiteral("Remote host closed connection");
    break;
  case QLowEnergyController::AuthorizationError:
    errorMsg = QStringLiteral("Authorization error");
    break;
  case QLowEnergyController::MissingPermissionsError:
    errorMsg = QStringLiteral("Missing permissions");
    break;
  case QLowEnergyController::RssiReadError:
    errorMsg = QStringLiteral("RSSI read error");
    break;
  }

  qWarning() << "BLE controller error:" << errorMsg;
  m_connecting = false;
  cleanup();
  emit connectionFailed(errorMsg);
}

void ConnectionManager::onMugReady() {
  if (m_mug != nullptr && m_mug->isReady()) {
    m_initTimer.stop();
    qInfo() << "Ember Mug ready";
    m_connecting = false;
    emit mugReady();
  }
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static) - Qt slot
void ConnectionManager::onMugError(const QString &error) {
  qWarning() << "Mug error:" << error;
  // Don't cleanup on transient errors, only log them
}

void ConnectionManager::onInitTimeout() {
  qWarning() << "Mug initialization timeout - service discovery may have "
                "failed";
  cleanup();
  emit connectionFailed(QStringLiteral("Initialization timeout"));
}

void ConnectionManager::onPairingFinished(
    const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing) {
  if (address != m_currentDevice.address()) {
    return;
  }

  if (pairing == QBluetoothLocalDevice::Unpaired) {
    qWarning() << "Pairing failed for device:" << address.toString();
    m_connecting = false;
    emit connectionFailed(QStringLiteral("Pairing failed"));
    return;
  }

  qInfo() << "Pairing successful for device:" << address.toString();
  proceedWithConnection();
}

void ConnectionManager::cleanup() {
  m_initTimer.stop();
  if (m_mug != nullptr) {
    m_mug->deleteLater();
    m_mug = nullptr;
  }
  if (m_controller != nullptr) {
    m_controller->deleteLater();
    m_controller = nullptr;
  }
}
