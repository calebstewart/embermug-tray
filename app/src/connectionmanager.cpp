#include "connectionmanager.h"
#include <QBluetoothAddress>
#include <QDebug>
#include <QSettings>

ConnectionManager::ConnectionManager(QObject *parent) : QObject(parent) {}

ConnectionManager::~ConnectionManager() { cleanup(); }

void ConnectionManager::connectToDevice(const QBluetoothDeviceInfo &device) {
  if (m_controller || m_connecting) {
    qWarning() << "Already connected or connecting";
    return;
  }

  m_connecting = true;
  m_currentDevice = device;
  emit connecting();

  m_controller = QLowEnergyController::createCentral(device, this);
  if (!m_controller) {
    m_connecting = false;
    emit connectionFailed(QStringLiteral("Failed to create BLE controller"));
    return;
  }

  connect(m_controller, &QLowEnergyController::connected, this,
          &ConnectionManager::onControllerConnected);
  connect(m_controller, &QLowEnergyController::disconnected, this,
          &ConnectionManager::onControllerDisconnected);
  connect(m_controller, &QLowEnergyController::errorOccurred, this,
          &ConnectionManager::onControllerError);

  qInfo() << "Connecting to device:" << device.name()
          << device.address().toString();
  m_controller->connectToDevice();
}

void ConnectionManager::disconnect() {
  if (m_controller) {
    m_controller->disconnectFromDevice();
  }
  cleanup();
}

bool ConnectionManager::isConnected() const {
  return m_controller &&
         m_controller->state() == QLowEnergyController::ConnectedState &&
         m_mug && m_mug->isReady();
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
  m_mug = new Ember::Mug(m_controller, this);
  connect(m_mug, &Ember::Mug::readyChanged, this,
          &ConnectionManager::onMugReady);
  connect(m_mug, &Ember::Mug::error, this, &ConnectionManager::onMugError);

  m_mug->initialize();
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
  if (m_mug && m_mug->isReady()) {
    qInfo() << "Ember Mug ready";
    m_connecting = false;
    emit mugReady();
  }
}

void ConnectionManager::onMugError(const QString &error) {
  qWarning() << "Mug error:" << error;
  // Don't cleanup on transient errors, only log them
}

void ConnectionManager::cleanup() {
  if (m_mug) {
    m_mug->deleteLater();
    m_mug = nullptr;
  }
  if (m_controller) {
    m_controller->deleteLater();
    m_controller = nullptr;
  }
}
