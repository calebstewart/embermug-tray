#include "constants.h"
#include "protocol.h"
#include "service.h"
#include <QDebug>
#include <ember/mug.h>

namespace Ember {

bool Mug::isEmberMug(const QBluetoothDeviceInfo &device) {
  if (device.name().startsWith(QStringLiteral("Ember"), Qt::CaseInsensitive)) {
    return true;
  }
  return device.serviceUuids().contains(serviceUuid());
}

QBluetoothUuid Mug::serviceUuid() {
  return QBluetoothUuid(QString::fromLatin1(Constants::SERVICE_UUID));
}

Mug::Mug(QLowEnergyController *controller, QObject *parent)
    : QObject(parent), m_controller(controller),
      m_service(new Service(controller, this)) {
  connect(m_service, &Service::ready, this, &Mug::onServiceReady);
  connect(m_service, &Service::error, this, &Mug::onServiceError);
  connect(m_service, &Service::dataReceived, this, &Mug::onDataReceived);
}

Mug::~Mug() {}

void Mug::initialize() {
  if (!m_controller) {
    emit error(QStringLiteral("No controller provided"));
    return;
  }

  if (m_controller->state() != QLowEnergyController::ConnectedState) {
    emit error(
        QStringLiteral("Controller must be connected before initializing"));
    return;
  }

  m_service->initialize();
}

bool Mug::isReady() const { return m_ready; }

float Mug::currentTemp() const { return m_currentTemp; }

float Mug::targetTemp() const { return m_targetTemp; }

TempUnit Mug::tempUnit() const { return m_tempUnit; }

int Mug::batteryLevel() const { return m_batteryLevel; }

BatteryState Mug::batteryState() const { return m_batteryState; }

LiquidState Mug::liquidState() const { return m_liquidState; }

bool Mug::isHeating() const { return m_isHeating; }

QString Mug::name() const { return m_name; }

void Mug::setTargetTemperature(float celsius) {
  float temp = qBound(Constants::MIN_TEMPERATURE_C, celsius,
                      Constants::MAX_TEMPERATURE_C);
  QByteArray command = Protocol::setTemperature(temp);
  sendCommand(command);
}

void Mug::setTemperatureUnit(TempUnit unit) {
  QByteArray command = Protocol::setTempUnit(unit);
  sendCommand(command);
}

void Mug::refresh() {
  QByteArray command = Protocol::getMugState();
  sendCommand(command);
}

void Mug::onServiceReady() {
  qInfo() << "Mug service ready";
  m_ready = true;
  emit readyChanged();

  // Request initial state
  refresh();
}

void Mug::onServiceError(const QString &errorMsg) {
  qWarning() << "Mug service error:" << errorMsg;
  emit error(errorMsg);
}

void Mug::onDataReceived(const QByteArray &data) { handleResponse(data); }

void Mug::sendCommand(const QByteArray &command) {
  if (!m_service || !m_service->isReady()) {
    qWarning() << "Cannot send command: service not ready";
    return;
  }

  m_service->writeCommand(command);
}

void Mug::handleResponse(const QByteArray &data) {
  auto response = Protocol::parseResponse(data);

  if (!response.error.isEmpty()) {
    qWarning() << "Response error:" << response.error;
    emit error(response.error);
    return;
  }

  switch (response.type) {
  case Protocol::ResponseType::Ack:
    qInfo() << "Command acknowledged";
    break;
  case Protocol::ResponseType::Nack:
    qWarning() << "Command failed (NACK)";
    emit error(QStringLiteral("Command rejected by device"));
    break;
  case Protocol::ResponseType::Data:
    // Parse mug state
    if (response.payload.size() >= 4) {
      // Current temperature (Celsius * 100, little-endian)
      int tempInt = static_cast<quint8>(response.payload[0]) |
                    (static_cast<quint8>(response.payload[1]) << 8);
      float newCurrentTemp = tempInt / 100.0f;
      if (!qFuzzyCompare(m_currentTemp, newCurrentTemp)) {
        m_currentTemp = newCurrentTemp;
        emit currentTempChanged();
      }

      // Target temperature
      int targetInt = static_cast<quint8>(response.payload[2]) |
                      (static_cast<quint8>(response.payload[3]) << 8);
      float newTargetTemp = targetInt / 100.0f;
      if (!qFuzzyCompare(m_targetTemp, newTargetTemp)) {
        m_targetTemp = newTargetTemp;
        emit targetTempChanged();
      }

      if (response.payload.size() >= 5) {
        int newBatteryLevel = static_cast<quint8>(response.payload[4]);
        if (m_batteryLevel != newBatteryLevel) {
          m_batteryLevel = newBatteryLevel;
          emit batteryLevelChanged();
        }
      }

      if (response.payload.size() >= 6) {
        // Flags byte - bit 0 = heating
        quint8 flags = static_cast<quint8>(response.payload[5]);
        bool newIsHeating = (flags & 0x01) != 0;
        if (m_isHeating != newIsHeating) {
          m_isHeating = newIsHeating;
          emit heatingChanged();
        }
      }

      emit stateUpdated();
    }
    break;
  }
}

} // namespace Ember
