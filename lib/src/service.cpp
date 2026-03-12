#include "service.h"
#include "constants.h"
#include <QDebug>

namespace Ember {

Service::Service(QLowEnergyController *controller, QObject *parent)
    : QObject(parent), m_controller(controller) {}

Service::~Service() {
  if (m_service) {
    m_service->deleteLater();
  }
}

void Service::initialize() {
  if (!m_controller) {
    emit error(QStringLiteral("No controller provided"));
    return;
  }

  if (m_controller->state() != QLowEnergyController::ConnectedState) {
    emit error(QStringLiteral("Controller not connected"));
    return;
  }

  connect(m_controller, &QLowEnergyController::serviceDiscovered, this,
          &Service::onServiceDiscovered);
  connect(m_controller, &QLowEnergyController::discoveryFinished, this,
          &Service::onServiceDiscoveryFinished);

  m_controller->discoverServices();
}

bool Service::isReady() const { return m_ready; }

bool Service::writeCommand(const QByteArray &data) {
  if (!m_service || !m_ready) {
    qWarning() << "Service not ready for writing";
    return false;
  }

  QBluetoothUuid writeUuid(
      QString::fromLatin1(Constants::CHARACTERISTIC_WRITE));
  auto chars = m_service->characteristics();
  for (const auto &chr : chars) {
    if (chr.uuid() == writeUuid) {
      m_service->writeCharacteristic(chr, data,
                                     QLowEnergyService::WriteWithoutResponse);
      return true;
    }
  }

  qWarning() << "Write characteristic not found";
  return false;
}

void Service::onServiceDiscovered(const QBluetoothUuid &uuid) {
  QBluetoothUuid emberServiceUuid(QString::fromLatin1(Constants::SERVICE_UUID));
  if (uuid == emberServiceUuid) {
    qInfo() << "Ember service discovered";
  }
}

void Service::onServiceDiscoveryFinished() {
  qInfo() << "Service discovery finished";

  QBluetoothUuid emberServiceUuid(QString::fromLatin1(Constants::SERVICE_UUID));
  m_service = m_controller->createServiceObject(emberServiceUuid, this);

  if (!m_service) {
    emit error(QStringLiteral("Ember service not found on device"));
    return;
  }

  connect(m_service, &QLowEnergyService::stateChanged, this,
          &Service::onServiceStateChanged);
  connect(m_service, &QLowEnergyService::characteristicChanged, this,
          &Service::onCharacteristicChanged);
  connect(m_service, &QLowEnergyService::errorOccurred, this,
          &Service::onServiceError);

  m_service->discoverDetails();
}

void Service::onServiceStateChanged(QLowEnergyService::ServiceState newState) {
  qInfo() << "Service state changed:" << newState;

  if (newState == QLowEnergyService::RemoteServiceDiscovered) {
    setupNotifications();
    m_ready = true;
    emit ready();
  }
}

void Service::onCharacteristicChanged(const QLowEnergyCharacteristic &charInfo,
                                      const QByteArray &value) {
  QBluetoothUuid notifyUuid(
      QString::fromLatin1(Constants::CHARACTERISTIC_NOTIFY));
  if (charInfo.uuid() == notifyUuid) {
    emit dataReceived(value);
  }
}

void Service::onServiceError(QLowEnergyService::ServiceError error) {
  QString errorMsg;
  switch (error) {
  case QLowEnergyService::NoError:
    return;
  case QLowEnergyService::OperationError:
    errorMsg = QStringLiteral("Operation error");
    break;
  case QLowEnergyService::CharacteristicWriteError:
    errorMsg = QStringLiteral("Characteristic write error");
    break;
  case QLowEnergyService::DescriptorWriteError:
    errorMsg = QStringLiteral("Descriptor write error");
    break;
  case QLowEnergyService::UnknownError:
    errorMsg = QStringLiteral("Unknown service error");
    break;
  case QLowEnergyService::CharacteristicReadError:
    errorMsg = QStringLiteral("Characteristic read error");
    break;
  case QLowEnergyService::DescriptorReadError:
    errorMsg = QStringLiteral("Descriptor read error");
    break;
  }
  emit this->error(errorMsg);
}

void Service::setupNotifications() {
  if (!m_service)
    return;

  QBluetoothUuid notifyUuid(
      QString::fromLatin1(Constants::CHARACTERISTIC_NOTIFY));
  auto chars = m_service->characteristics();

  for (const auto &chr : chars) {
    if (chr.uuid() == notifyUuid) {
      auto descriptor = chr.descriptor(
          QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
      if (descriptor.isValid()) {
        m_service->writeDescriptor(descriptor, QByteArray::fromHex("0100"));
        qInfo() << "Notifications enabled on characteristic";
      }
      break;
    }
  }
}

} // namespace Ember
