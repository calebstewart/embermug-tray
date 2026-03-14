#include "constants.h"
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
  // NOLINTNEXTLINE(modernize-return-braced-init-list) - QBluetoothUuid has explicit constructor
  return QBluetoothUuid(QString::fromLatin1(Constants::SERVICE_UUID));
}

Mug::Mug(QLowEnergyController *controller, QObject *parent)
    : QObject(parent), m_controller(controller),
      m_service(new Service(controller, this)) {
  connect(m_service, &Service::ready, this, &Mug::onServiceReady);
  connect(m_service, &Service::error, this, &Mug::onServiceError);
  connect(m_service, &Service::mugNameReceived, this, &Mug::onMugNameReceived);
  connect(m_service, &Service::currentTempReceived, this,
          &Mug::onCurrentTempReceived);
  connect(m_service, &Service::targetTempReceived, this,
          &Mug::onTargetTempReceived);
  connect(m_service, &Service::tempUnitReceived, this,
          &Mug::onTempUnitReceived);
  connect(m_service, &Service::batteryReceived, this, &Mug::onBatteryReceived);
  connect(m_service, &Service::liquidStateReceived, this,
          &Mug::onLiquidStateReceived);
  connect(m_service, &Service::pushEventReceived, this,
          &Mug::onPushEventReceived);
  connect(m_service, &Service::colorReceived, this, &Mug::onColorReceived);
}

Mug::~Mug() = default;

void Mug::initialize() {
  if (m_controller == nullptr) {
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

MugColor Mug::color() const { return m_color; }

void Mug::setTargetTemperature(float celsius) {
  if (m_service == nullptr || !m_service->isReady()) {
    qWarning() << "Cannot set target temperature: service not ready";
    return;
  }

  float temp = qBound(Constants::MIN_TEMPERATURE_C, celsius,
                      Constants::MAX_TEMPERATURE_C);
  auto tempValue = static_cast<quint16>(temp * 100);
  m_service->writeTargetTemp(tempValue);
}

void Mug::setTemperatureUnit(TempUnit unit) {
  if (m_service == nullptr || !m_service->isReady()) {
    qWarning() << "Cannot set temperature unit: service not ready";
    return;
  }

  m_service->writeTempUnit(static_cast<quint8>(unit));
}

void Mug::setColor(const MugColor &color) {
  if (m_service == nullptr || !m_service->isReady()) {
    qWarning() << "Cannot set color: service not ready";
    return;
  }

  m_service->writeColor(color.red, color.green, color.blue, color.alpha);
}

void Mug::refresh() {
  if (m_service == nullptr || !m_service->isReady()) {
    qWarning() << "Cannot refresh: service not ready";
    return;
  }

  m_service->readCurrentTemp();
  m_service->readTargetTemp();
  m_service->readBattery();
  m_service->readLiquidState();
  m_service->readColor();
}

void Mug::onServiceReady() {
  qInfo() << "Mug service ready";
  m_ready = true;
  emit readyChanged();
}

void Mug::onServiceError(const QString &errorMsg) {
  qWarning() << "Mug service error:" << errorMsg;
  emit error(errorMsg);
}

void Mug::onMugNameReceived(const QString &name) {
  if (m_name != name) {
    m_name = name;
    emit nameChanged();
    emit stateUpdated();
  }
}

void Mug::onCurrentTempReceived(float tempCelsius) {
  if (!qFuzzyCompare(m_currentTemp, tempCelsius)) {
    m_currentTemp = tempCelsius;
    emit currentTempChanged();
    emit stateUpdated();
  }
}

void Mug::onTargetTempReceived(float tempCelsius) {
  if (!qFuzzyCompare(m_targetTemp, tempCelsius)) {
    m_targetTemp = tempCelsius;
    emit targetTempChanged();
    emit stateUpdated();
  }
}

void Mug::onTempUnitReceived(quint8 unit) {
  TempUnit newUnit = (unit == 0) ? TempUnit::Celsius : TempUnit::Fahrenheit;
  if (m_tempUnit != newUnit) {
    m_tempUnit = newUnit;
    emit tempUnitChanged();
    emit stateUpdated();
  }
}

void Mug::onBatteryReceived(int level, bool charging) {
  bool changed = false;

  if (m_batteryLevel != level) {
    m_batteryLevel = level;
    emit batteryLevelChanged();
    changed = true;
  }

  BatteryState newState =
      charging ? BatteryState::Charging : BatteryState::Discharging;
  if (m_batteryState != newState) {
    m_batteryState = newState;
    emit batteryStateChanged();
    changed = true;
  }

  if (changed) {
    emit stateUpdated();
  }
}

void Mug::onLiquidStateReceived(quint8 state) {
  auto newState = static_cast<LiquidState>(state);
  if (m_liquidState != newState) {
    m_liquidState = newState;
    emit liquidStateChanged();

    // Update heating state based on liquid state
    bool newIsHeating = (newState == LiquidState::Heating);
    if (m_isHeating != newIsHeating) {
      m_isHeating = newIsHeating;
      emit heatingChanged();
    }

    emit stateUpdated();
  }
}

void Mug::onPushEventReceived(quint8 event) {
  qInfo() << "Push event received:" << event;

  // React to push events by reading relevant characteristics
  switch (event) {
  case Constants::EVENT_REFRESH_BATTERY:
  case Constants::EVENT_CHARGING:
  case Constants::EVENT_NOT_CHARGING:
    m_service->readBattery();
    break;
  case Constants::EVENT_REFRESH_TARGET_TEMP:
    m_service->readTargetTemp();
    break;
  case Constants::EVENT_REFRESH_CURRENT_TEMP:
    m_service->readCurrentTemp();
    break;
  case Constants::EVENT_REFRESH_LIQUID_LEVEL:
  case Constants::EVENT_REFRESH_LIQUID_STATE:
    m_service->readLiquidState();
    break;
  default:
    // Refresh everything for unknown events
    refresh();
    break;
  }
}

void Mug::onColorReceived(quint8 red, quint8 green, quint8 blue, quint8 alpha) {
  MugColor newColor{red, green, blue, alpha};
  if (m_color != newColor) {
    m_color = newColor;
    emit colorChanged();
    emit stateUpdated();
  }
}

} // namespace Ember
