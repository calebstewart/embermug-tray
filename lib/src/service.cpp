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

QLowEnergyCharacteristic Service::findCharacteristic(const char *uuid) {
  if (!m_service) {
    return QLowEnergyCharacteristic();
  }

  QBluetoothUuid charUuid(QString::fromLatin1(uuid));
  auto chars = m_service->characteristics();
  for (const auto &chr : chars) {
    if (chr.uuid() == charUuid) {
      return chr;
    }
  }
  return QLowEnergyCharacteristic();
}

void Service::readMugName() {
  auto chr = findCharacteristic(Constants::CHAR_MUG_NAME);
  if (chr.isValid()) {
    m_service->readCharacteristic(chr);
  }
}

void Service::readCurrentTemp() {
  auto chr = findCharacteristic(Constants::CHAR_CURRENT_TEMP);
  if (chr.isValid()) {
    m_service->readCharacteristic(chr);
  }
}

void Service::readTargetTemp() {
  auto chr = findCharacteristic(Constants::CHAR_TARGET_TEMP);
  if (chr.isValid()) {
    m_service->readCharacteristic(chr);
  }
}

void Service::readTempUnit() {
  auto chr = findCharacteristic(Constants::CHAR_TEMP_UNIT);
  if (chr.isValid()) {
    m_service->readCharacteristic(chr);
  }
}

void Service::readBattery() {
  auto chr = findCharacteristic(Constants::CHAR_BATTERY);
  if (chr.isValid()) {
    m_service->readCharacteristic(chr);
  }
}

void Service::readLiquidState() {
  auto chr = findCharacteristic(Constants::CHAR_LIQUID_STATE);
  if (chr.isValid()) {
    m_service->readCharacteristic(chr);
  }
}

void Service::writeTargetTemp(quint16 tempCelsiusTimes100) {
  auto chr = findCharacteristic(Constants::CHAR_TARGET_TEMP);
  if (chr.isValid()) {
    QByteArray data;
    data.append(static_cast<char>(tempCelsiusTimes100 & 0xFF));
    data.append(static_cast<char>((tempCelsiusTimes100 >> 8) & 0xFF));

    // Use WriteWithoutResponse if the characteristic supports it
    auto mode = (chr.properties() & QLowEnergyCharacteristic::WriteNoResponse)
                    ? QLowEnergyService::WriteWithoutResponse
                    : QLowEnergyService::WriteWithResponse;
    m_service->writeCharacteristic(chr, data, mode);
  } else {
    qWarning() << "Target temp characteristic not found";
  }
}

void Service::writeTempUnit(quint8 unit) {
  auto chr = findCharacteristic(Constants::CHAR_TEMP_UNIT);
  if (chr.isValid()) {
    QByteArray data;
    data.append(static_cast<char>(unit));

    // Use WriteWithoutResponse if the characteristic supports it
    auto mode = (chr.properties() & QLowEnergyCharacteristic::WriteNoResponse)
                    ? QLowEnergyService::WriteWithoutResponse
                    : QLowEnergyService::WriteWithResponse;
    m_service->writeCharacteristic(chr, data, mode);
  } else {
    qWarning() << "Temp unit characteristic not found";
  }
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
  connect(m_service, &QLowEnergyService::characteristicRead, this,
          &Service::onCharacteristicRead);
  connect(m_service, &QLowEnergyService::characteristicChanged, this,
          &Service::onCharacteristicChanged);
  connect(m_service, &QLowEnergyService::errorOccurred, this,
          &Service::onServiceError);

  m_service->discoverDetails();
}

void Service::onServiceStateChanged(QLowEnergyService::ServiceState newState) {
  qInfo() << "Service state changed:" << newState;

  if (newState == QLowEnergyService::RemoteServiceDiscovered) {
    qInfo() << "Characteristics discovered:";
    for (const auto &chr : m_service->characteristics()) {
      qInfo() << "  -" << chr.uuid().toString();
    }

    setupNotifications();
    m_ready = true;
    emit ready();

    // Read all initial values
    readAllCharacteristics();
  }
}

void Service::onCharacteristicRead(const QLowEnergyCharacteristic &charInfo,
                                   const QByteArray &value) {
  QBluetoothUuid mugNameUuid(QString::fromLatin1(Constants::CHAR_MUG_NAME));
  QBluetoothUuid currentTempUuid(
      QString::fromLatin1(Constants::CHAR_CURRENT_TEMP));
  QBluetoothUuid targetTempUuid(
      QString::fromLatin1(Constants::CHAR_TARGET_TEMP));
  QBluetoothUuid tempUnitUuid(QString::fromLatin1(Constants::CHAR_TEMP_UNIT));
  QBluetoothUuid batteryUuid(QString::fromLatin1(Constants::CHAR_BATTERY));
  QBluetoothUuid liquidStateUuid(
      QString::fromLatin1(Constants::CHAR_LIQUID_STATE));

  if (charInfo.uuid() == mugNameUuid) {
    QString name = QString::fromUtf8(value);
    emit mugNameReceived(name);
  } else if (charInfo.uuid() == currentTempUuid) {
    if (value.size() >= 2) {
      quint16 tempInt = static_cast<quint8>(value[0]) |
                        (static_cast<quint8>(value[1]) << 8);
      float tempCelsius = tempInt * 0.01f;
      emit currentTempReceived(tempCelsius);
    }
  } else if (charInfo.uuid() == targetTempUuid) {
    if (value.size() >= 2) {
      quint16 tempInt = static_cast<quint8>(value[0]) |
                        (static_cast<quint8>(value[1]) << 8);
      float tempCelsius = tempInt * 0.01f;
      emit targetTempReceived(tempCelsius);
    }
  } else if (charInfo.uuid() == tempUnitUuid) {
    if (value.size() >= 1) {
      emit tempUnitReceived(static_cast<quint8>(value[0]));
    }
  } else if (charInfo.uuid() == batteryUuid) {
    if (value.size() >= 2) {
      int level = static_cast<quint8>(value[0]);
      bool charging = static_cast<quint8>(value[1]) == 1;
      emit batteryReceived(level, charging);
    }
  } else if (charInfo.uuid() == liquidStateUuid) {
    if (value.size() >= 1) {
      emit liquidStateReceived(static_cast<quint8>(value[0]));
    }
  }
}

void Service::onCharacteristicChanged(const QLowEnergyCharacteristic &charInfo,
                                      const QByteArray &value) {
  QBluetoothUuid pushEventsUuid(
      QString::fromLatin1(Constants::CHAR_PUSH_EVENTS));

  if (charInfo.uuid() == pushEventsUuid) {
    if (value.size() >= 1) {
      emit pushEventReceived(static_cast<quint8>(value[0]));
    }
  } else {
    // Treat other characteristic changes like reads
    onCharacteristicRead(charInfo, value);
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

  // Enable notifications on push events characteristic
  auto chr = findCharacteristic(Constants::CHAR_PUSH_EVENTS);
  if (chr.isValid()) {
    auto descriptor = chr.descriptor(
        QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
    if (descriptor.isValid()) {
      m_service->writeDescriptor(descriptor, QByteArray::fromHex("0100"));
      qInfo() << "Push event notifications enabled";
    }
  } else {
    qWarning() << "Push events characteristic not found";
  }
}

void Service::readAllCharacteristics() {
  readMugName();
  readCurrentTemp();
  readTargetTemp();
  readTempUnit();
  readBattery();
  readLiquidState();
}

} // namespace Ember
