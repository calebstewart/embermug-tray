#include "traycontroller.h"
#include "connectionmanager.h"
#include "devicemonitor.h"
#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QPainter>
#include <QSvgRenderer>

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

  m_contextMenu->addSeparator();

  // Scan action
  m_scanAction = new QAction(QStringLiteral("Scan for Devices"), m_contextMenu);
  connect(m_scanAction, &QAction::triggered, m_monitor,
          &DeviceMonitor::startMonitoring);
  m_contextMenu->addAction(m_scanAction);

  // Devices submenu
  m_devicesMenu = new QMenu(QStringLiteral("Connect to Device"), m_contextMenu);
  m_devicesMenu->setEnabled(false);
  m_contextMenu->addMenu(m_devicesMenu);

  m_contextMenu->addSeparator();

  // Disconnect action
  m_disconnectAction = new QAction(QStringLiteral("Disconnect"), m_contextMenu);
  m_disconnectAction->setEnabled(false);
  connect(m_disconnectAction, &QAction::triggered, m_manager,
          &ConnectionManager::disconnect);
  m_contextMenu->addAction(m_disconnectAction);

  // Quit action
  m_quitAction = new QAction(QStringLiteral("Quit"), m_contextMenu);
  connect(m_quitAction, &QAction::triggered, qApp, &QApplication::quit);
  m_contextMenu->addAction(m_quitAction);

  m_trayIcon->setContextMenu(m_contextMenu);

  QSvgRenderer renderer(QStringLiteral(":/coffee.svg"));
  QPixmap pixmap(22, 22);
  pixmap.fill(Qt::transparent);
  QPainter painter(&pixmap);
  renderer.render(&painter);
  m_trayIcon->setIcon(QIcon(pixmap));

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

  // Start monitoring for devices
  m_monitor->startMonitoring();
}

void TrayController::onDevicesChanged() { rebuildDeviceMenu(); }

void TrayController::onScanStarted() {
  m_scanAction->setText(QStringLiteral("Scanning..."));
  m_scanAction->setEnabled(false);
}

void TrayController::onScanFinished() {
  m_scanAction->setText(QStringLiteral("Scan for Devices"));
  m_scanAction->setEnabled(true);
  rebuildDeviceMenu();
}

void TrayController::onConnecting() {
  m_statusAction->setText(QStringLiteral("Status: Connecting..."));
  m_devicesMenu->setEnabled(false);
}

void TrayController::onConnected() {
  m_statusAction->setText(QStringLiteral("Status: Connected"));
}

void TrayController::onDisconnected() {
  m_statusAction->setText(QStringLiteral("Status: Not connected"));
  m_tempAction->setText(QStringLiteral("Temp: --"));
  m_batteryAction->setText(QStringLiteral("Battery: --"));
  m_disconnectAction->setEnabled(false);
  m_refreshTimer.stop();
  rebuildDeviceMenu();
  updateTrayTooltip();
}

void TrayController::onMugReady() {
  Ember::Mug *mug = m_manager->mug();
  if (!mug)
    return;

  QString name = mug->name();
  if (name.isEmpty()) {
    name = QStringLiteral("Ember Mug");
  }
  m_statusAction->setText(QStringLiteral("Status: Connected to %1").arg(name));
  m_disconnectAction->setEnabled(true);

  connect(mug, &Ember::Mug::stateUpdated, this,
          &TrayController::onMugStateUpdated);

  updateMugDisplay();
  m_refreshTimer.start();
}

void TrayController::onConnectionFailed(const QString &error) {
  m_statusAction->setText(QStringLiteral("Status: Connection failed"));
  qWarning() << "Connection failed:" << error;
  rebuildDeviceMenu();
}

void TrayController::onMugStateUpdated() { updateMugDisplay(); }

void TrayController::onRefreshTimer() {
  Ember::Mug *mug = m_manager->mug();
  if (mug && mug->isReady()) {
    mug->refresh();
  }
}

void TrayController::rebuildDeviceMenu() {
  m_devicesMenu->clear();

  QList<QBluetoothDeviceInfo> devices = m_monitor->availableDevices();

  if (devices.isEmpty()) {
    m_devicesMenu->setTitle(QStringLiteral("No devices found"));
    m_devicesMenu->setEnabled(false);
    return;
  }

  m_devicesMenu->setTitle(
      QStringLiteral("Connect (%1 found)").arg(devices.size()));
  m_devicesMenu->setEnabled(!m_manager->isConnected() &&
                            !m_manager->isConnecting());

  for (const auto &device : devices) {
    QString displayName = device.name();
    if (displayName.isEmpty()) {
      displayName = device.address().toString();
    }

    QAction *action = m_devicesMenu->addAction(displayName);
    connect(action, &QAction::triggered, this,
            [this, device]() { m_manager->connectToDevice(device); });
  }
}

void TrayController::updateMugDisplay() {
  Ember::Mug *mug = m_manager->mug();
  if (!mug)
    return;

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
}

void TrayController::updateTrayTooltip() {
  QString tooltip = QStringLiteral("EmberMug Tray\n");

  Ember::Mug *mug = m_manager->mug();
  if (mug && mug->isReady()) {
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
    tooltip += QStringLiteral("Status: Not connected\n");
    tooltip +=
        QStringLiteral("Click 'Scan for Devices' to find your Ember Mug");
  }

  m_trayIcon->setToolTip(tooltip);
}
