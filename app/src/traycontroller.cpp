#include "traycontroller.h"
#include "connectionmanager.h"
#include "devicemonitor.h"
#include <QApplication>
#include <QBuffer>
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QPainter>
#include <QRegularExpression>
#include <QSvgRenderer>
#include <array>
#include <cmath>

TrayController::TrayController(DeviceMonitor *monitor,
                               ConnectionManager *manager, QObject *parent)
    : QObject(parent), m_monitor(monitor), m_manager(manager),
      m_trayIcon(new QSystemTrayIcon(this)), m_contextMenu(new QMenu()) {
  // Status action (non-interactive)
  m_statusAction =
      new QAction(QStringLiteral("Status: Not connected"), m_contextMenu);
  m_statusAction->setEnabled(false);
  m_contextMenu->addAction(m_statusAction);

  // Temperature display
  m_tempAction = new QAction(QStringLiteral("Temp: --"), m_contextMenu);
  m_tempAction->setEnabled(false);
  m_contextMenu->addAction(m_tempAction);

  // Battery display
  m_batteryAction = new QAction(QStringLiteral("Battery: --"), m_contextMenu);
  m_batteryAction->setEnabled(false);
  m_contextMenu->addAction(m_batteryAction);

  // Set target temperature submenu
  m_targetTempMenu =
      new QMenu(QStringLiteral("Set Target"), m_contextMenu);
  m_targetTempMenu->setEnabled(false); // Disabled until connected
  m_contextMenu->addMenu(m_targetTempMenu);

  m_contextMenu->addSeparator();

  // Devices submenu - always titled "Switch Device"
  m_devicesMenu = new QMenu(QStringLiteral("Switch Device"), m_contextMenu);
  m_contextMenu->addMenu(m_devicesMenu);

  // Start scanning when menu opens, stop when it closes
  connect(m_contextMenu, &QMenu::aboutToShow, this, [this]() {
    m_monitor->startMonitoring();
  });
  connect(m_contextMenu, &QMenu::aboutToHide, this, [this]() {
    m_monitor->stopMonitoring();
  });

  m_contextMenu->addSeparator();

  // Quit action
  m_quitAction = new QAction(QStringLiteral("Quit"), m_contextMenu);
  connect(m_quitAction, &QAction::triggered, qApp, &QApplication::quit);
  m_contextMenu->addAction(m_quitAction);

  m_trayIcon->setContextMenu(m_contextMenu);

  initIconCache();
  updateTrayIcon();

  // Connect monitor signals
  connect(m_monitor, &DeviceMonitor::devicesChanged, this,
          &TrayController::onDevicesChanged);
  connect(m_monitor, &DeviceMonitor::scanStarted, this,
          &TrayController::onScanStarted);
  connect(m_monitor, &DeviceMonitor::scanFinished, this,
          &TrayController::onScanFinished);

  // Connect manager signals
  connect(m_manager, &ConnectionManager::connecting, this,
          &TrayController::onConnecting);
  connect(m_manager, &ConnectionManager::connected, this,
          &TrayController::onConnected);
  connect(m_manager, &ConnectionManager::disconnected, this,
          &TrayController::onDisconnected);
  connect(m_manager, &ConnectionManager::mugReady, this,
          &TrayController::onMugReady);
  connect(m_manager, &ConnectionManager::connectionFailed, this,
          &TrayController::onConnectionFailed);

  // Refresh timer for mug state
  m_refreshTimer.setInterval(2000);
  connect(&m_refreshTimer, &QTimer::timeout, this,
          &TrayController::onRefreshTimer);
}

TrayController::~TrayController() { delete m_contextMenu; }

void TrayController::show() {
  m_trayIcon->setVisible(true);
  updateTrayTooltip();

  // Load device preferences from settings
  m_preferredAddress = m_settings.value(QStringLiteral("device.address")).toString();
  m_preferredName = m_settings.value(QStringLiteral("device.name")).toString();

  // Start scanning for devices
  m_monitor->startMonitoring();
}

void TrayController::onDevicesChanged() {
  rebuildDeviceMenu();

  // Don't change connection if already connected or connecting
  if (m_manager->isConnected() || m_manager->isConnecting()) {
    return;
  }

  QList<QBluetoothDeviceInfo> mugs = m_monitor->availableDevices();

  // Priority 1: Check for preferred address
  if (!m_preferredAddress.isEmpty()) {
    auto device = m_monitor->findByAddress(QBluetoothAddress(m_preferredAddress));
    if (device) {
      m_manager->connectToDevice(*device);
      return;
    }
  }

  // Priority 2: Check for preferred name
  if (!m_preferredName.isEmpty()) {
    auto device = m_monitor->findByName(m_preferredName);
    if (device) {
      m_manager->connectToDevice(*device);
      return;
    }
  }

  // Priority 3: First valid Ember mug
  if (!mugs.isEmpty()) {
    m_manager->connectToDevice(mugs.first());
  }
  // Else: stay disconnected
}

void TrayController::onScanStarted() { rebuildDeviceMenu(); }

void TrayController::onScanFinished() { rebuildDeviceMenu(); }

void TrayController::onConnecting() {
  m_statusAction->setText(QStringLiteral("Status: Connecting..."));
}

void TrayController::onConnected() {
  m_statusAction->setText(QStringLiteral("Status: Connected"));
}

void TrayController::onDisconnected() {
  m_statusAction->setText(QStringLiteral("Status: Not connected"));
  m_tempAction->setText(QStringLiteral("Temp: --"));
  m_batteryAction->setText(QStringLiteral("Battery: --"));
  m_targetTempMenu->setEnabled(false);
  m_refreshTimer.stop();
  rebuildDeviceMenu();
  updateTrayTooltip();
  updateTrayIcon();
}

void TrayController::onMugReady() {
  Ember::Mug *mug = m_manager->mug();
  if (mug == nullptr) {
    return;
  }

  QString name = mug->name();
  if (name.isEmpty()) {
    name = QStringLiteral("Ember Mug");
  }
  m_statusAction->setText(QStringLiteral("Status: Connected to %1").arg(name));

  connect(mug, &Ember::Mug::stateUpdated, this,
          &TrayController::onMugStateUpdated);

  // Enable target temp menu and build presets
  m_targetTempMenu->setEnabled(true);
  rebuildTargetTempMenu();

  updateMugDisplay();
  updateTrayIcon();
  rebuildDeviceMenu();
  m_refreshTimer.start();
}

void TrayController::onConnectionFailed(const QString &error) {
  m_statusAction->setText(QStringLiteral("Status: Connection failed"));
  qWarning() << "Connection failed:" << error;
  rebuildDeviceMenu();
}

void TrayController::onMugStateUpdated() {
  updateMugDisplay();
  rebuildTargetTempMenu();
}

void TrayController::onRefreshTimer() {
  Ember::Mug *mug = m_manager->mug();
  if (mug != nullptr && mug->isReady()) {
    mug->refresh();
  }
}

void TrayController::rebuildDeviceMenu() {
  m_devicesMenu->clear();

  QList<QBluetoothDeviceInfo> devices = m_monitor->availableDevices();

  if (m_monitor->isScanning()) {
    auto *placeholder =
        m_devicesMenu->addAction(QStringLiteral("Looking for nearby mugs..."));
    placeholder->setEnabled(false);

    // Also show any devices found so far
    for (const auto &device : devices) {
      QString displayName = device.name();
      if (displayName.isEmpty()) {
        displayName = device.address().toString();
      }

      QAction *action = m_devicesMenu->addAction(displayName);
      action->setCheckable(true);

      // Mark currently connected device
      if (m_manager->isConnected() &&
          m_manager->connectedDevice().address() == device.address()) {
        action->setChecked(true);
      }

      connect(action, &QAction::triggered, this,
              [this, device]() {
                // Save device preference and connect
                m_settings.setValue(QStringLiteral("device.address"),
                                    device.address().toString());
                m_preferredAddress = device.address().toString();
                m_manager->disconnect();
                m_manager->connectToDevice(device);
              });
    }
    return;
  }

  if (devices.isEmpty()) {
    auto *placeholder =
        m_devicesMenu->addAction(QStringLiteral("No ember mugs found"));
    placeholder->setEnabled(false);
    return;
  }

  for (const auto &device : devices) {
    QString displayName = device.name();
    if (displayName.isEmpty()) {
      displayName = device.address().toString();
    }

    QAction *action = m_devicesMenu->addAction(displayName);
    action->setCheckable(true);

    // Mark currently connected device
    if (m_manager->isConnected() &&
        m_manager->connectedDevice().address() == device.address()) {
      action->setChecked(true);
    }

    connect(action, &QAction::triggered, this,
            [this, device]() {
              // Save device preference and connect
              m_settings.setValue(QStringLiteral("device.address"),
                                  device.address().toString());
              m_preferredAddress = device.address().toString();
              m_manager->disconnect();
              m_manager->connectToDevice(device);
            });
  }
}

void TrayController::updateMugDisplay() {
  Ember::Mug *mug = m_manager->mug();
  if (mug == nullptr) {
    return;
  }

  QString tempSuffix = (mug->tempUnit() == Ember::TempUnit::Celsius)
                           ? QStringLiteral("\u00B0C")
                           : QStringLiteral("\u00B0F");
  float displayTemp = mug->currentTemp();
  if (mug->tempUnit() == Ember::TempUnit::Fahrenheit) {
    displayTemp = Ember::celsiusToFahrenheit(displayTemp);
  }

  m_tempAction->setText(
      QStringLiteral("Temp: %1%2%3")
          .arg(QString::number(displayTemp, 'f', 1))
          .arg(tempSuffix)
          .arg(mug->isHeating() ? QStringLiteral(" (Heating)") : QString()));

  m_batteryAction->setText(
      QStringLiteral("Battery: %1% (%2)")
          .arg(mug->batteryLevel())
          .arg(Ember::batteryStateToString(mug->batteryState())));

  updateTrayTooltip();
  updateTrayIcon();
}

void TrayController::updateTrayTooltip() {
  QString tooltip = QStringLiteral("EmberMug Tray\n");

  Ember::Mug *mug = m_manager->mug();
  if (mug != nullptr && mug->isReady()) {
    QString tempSuffix = (mug->tempUnit() == Ember::TempUnit::Celsius)
                             ? QStringLiteral("\u00B0C")
                             : QStringLiteral("\u00B0F");
    float currentTemp = mug->currentTemp();
    float targetTemp = mug->targetTemp();
    if (mug->tempUnit() == Ember::TempUnit::Fahrenheit) {
      currentTemp = Ember::celsiusToFahrenheit(currentTemp);
      targetTemp = Ember::celsiusToFahrenheit(targetTemp);
    }

    tooltip += QStringLiteral("Status: Connected\n");
    tooltip += QStringLiteral("Current Temp: %1%2\n")
                   .arg(QString::number(currentTemp, 'f', 1))
                   .arg(tempSuffix);
    tooltip += QStringLiteral("Target Temp: %1%2\n")
                   .arg(QString::number(targetTemp, 'f', 1))
                   .arg(tempSuffix);
    tooltip += QStringLiteral("Battery: %1% (%2)")
                   .arg(mug->batteryLevel())
                   .arg(Ember::batteryStateToString(mug->batteryState()));
  } else {
    tooltip += QStringLiteral("Status: Not connected");
  }

  m_trayIcon->setToolTip(tooltip);
}

void TrayController::rebuildTargetTempMenu() {
  m_targetTempMenu->clear();

  Ember::Mug *mug = m_manager->mug();
  if (mug == nullptr || !mug->isReady()) {
    return;
  }

  // Temperature presets: name, celsius value, fahrenheit value
  struct TempPreset {
    const char *name;
    float celsius;
    int fahrenheit;
  };

  static const std::array<TempPreset, 4> presets = {{
      {"Extra Hot", 62.5f, 145},
      {"Coffee", 57.0f, 135},
      {"Tea", 54.0f, 130},
      {"Warm", 51.0f, 124},
  }};

  float currentTarget = mug->targetTemp();

  for (const auto &preset : presets) {
    QString label = QStringLiteral("%1 (%2 \u00B0C / %3 \u00B0F)")
                        .arg(QString::fromLatin1(preset.name))
                        .arg(preset.celsius, 0, 'f', 1)
                        .arg(preset.fahrenheit);

    QAction *action = m_targetTempMenu->addAction(label);
    action->setCheckable(true);

    // Check if this preset matches current target (within 0.5°C tolerance)
    if (std::fabs(currentTarget - preset.celsius) < 0.5f) {
      action->setChecked(true);
    }

    float presetCelsius = preset.celsius;
    connect(action, &QAction::triggered, this, [this, presetCelsius]() {
      Ember::Mug *innerMug = m_manager->mug();
      if (innerMug != nullptr && innerMug->isReady()) {
        innerMug->setTargetTemperature(presetCelsius);
      }
    });
  }
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
QIcon TrayController::renderIcon(const QColor &mugColor,
                                 const QColor &plateColor) {
  QFile file(QStringLiteral(":/coffee.svg"));
  if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << "Failed to open coffee.svg";
    // NOLINTNEXTLINE(modernize-return-braced-init-list) - explicit constructor
    return QIcon();
  }
  QString svgContent = QString::fromUtf8(file.readAll());
  file.close();

  // Replace fill colors for mug and plate paths
  static QRegularExpression mugFillRe(
      QStringLiteral(R"((<path[^>]*id="mug"[^>]*fill=")#[0-9a-fA-F]{6}("))"));
  static QRegularExpression plateFillRe(
      QStringLiteral(R"((<path[^>]*id="plate"[^>]*fill=")#[0-9a-fA-F]{6}("))"));

  svgContent.replace(mugFillRe,
                     QStringLiteral("\\1%1\\2").arg(mugColor.name()));
  svgContent.replace(plateFillRe,
                     QStringLiteral("\\1%1\\2").arg(plateColor.name()));

  QByteArray svgData = svgContent.toUtf8();
  QSvgRenderer renderer(svgData);

  QPixmap pixmap(22, 22);
  pixmap.fill(Qt::transparent);
  QPainter painter(&pixmap);
  renderer.render(&painter);

  // NOLINTNEXTLINE(modernize-return-braced-init-list) - explicit constructor
  return QIcon(pixmap);
}

void TrayController::initIconCache() {
  const std::array<QColor, 2> mugColors = {
      QColor(QStringLiteral("#e8e8e8")), // Normal
      QColor(QStringLiteral("#ff8844"))  // LowBattery
  };
  const std::array<QColor, 4> plateColors = {
      QColor(QStringLiteral("#888888")), // Gray
      QColor(QStringLiteral("#ff4444")), // Red (heating)
      QColor(QStringLiteral("#44cc44")), // Green (at temp)
      QColor(QStringLiteral("#4488ff"))  // Blue (cold)
  };

  for (size_t ms = 0; ms < mugColors.size(); ++ms) {
    for (size_t ps = 0; ps < plateColors.size(); ++ps) {
      m_iconCache.at(ms).at(ps) = renderIcon(mugColors.at(ms), plateColors.at(ps));
    }
  }
}

void TrayController::updateTrayIcon() {
  MugState mugState = MugState::Normal;
  PlateState plateState = PlateState::Gray;

  Ember::Mug *mug = m_manager->mug();
  if (mug != nullptr && mug->isReady()) {
    // Battery state
    if (mug->batteryLevel() < 20) {
      mugState = MugState::LowBattery;
    }

    // Plate state based on liquid state
    switch (mug->liquidState()) {
    case Ember::LiquidState::Empty:
    case Ember::LiquidState::Unknown:
      plateState = PlateState::Gray;
      break;
    case Ember::LiquidState::Heating:
      plateState = PlateState::Red;
      break;
    case Ember::LiquidState::AtTarget:
      plateState = PlateState::Green;
      break;
    case Ember::LiquidState::Filling:
    case Ember::LiquidState::Cold:
    case Ember::LiquidState::Cooling:
      plateState = PlateState::Blue;
      break;
    }
  }

  m_trayIcon->setIcon(
      m_iconCache.at(static_cast<size_t>(mugState)).at(static_cast<size_t>(plateState)));
}
