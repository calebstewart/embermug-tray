#ifndef TRAYCONTROLLER_H
#define TRAYCONTROLLER_H

#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QObject>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QTimer>
#include <array>
#include <ember/ember.h>

enum class PlateState { Gray, Red, Green, Blue };
enum class MugState { Normal, LowBattery };

class DeviceMonitor;
class ConnectionManager;

class TrayController : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TrayController)

public:
  explicit TrayController(DeviceMonitor *monitor, ConnectionManager *manager,
                          QObject *parent = nullptr);
  ~TrayController() override;

  void show();

private slots:
  void onDevicesChanged();
  void onScanStarted();
  void onScanFinished();
  void onConnecting();
  void onConnected();
  void onDisconnected();
  void onMugReady();
  void onConnectionFailed(const QString &error);
  void onMugStateUpdated();
  void onRefreshTimer();

private:
  void rebuildDeviceMenu();
  void rebuildTargetTempMenu();
  void updateMugDisplay();
  void updateTrayTooltip();
  QIcon renderIcon(const QColor &mugColor, const QColor &plateColor);
  void initIconCache();
  void updateTrayIcon();

  DeviceMonitor *m_monitor;
  ConnectionManager *m_manager;

  QSystemTrayIcon *m_trayIcon;
  QMenu *m_contextMenu;
  QMenu *m_devicesMenu;
  QAction *m_statusAction;
  QAction *m_tempAction;
  QAction *m_batteryAction;
  QMenu *m_targetTempMenu;
  QAction *m_quitAction;

  QTimer m_refreshTimer;
  QSettings m_settings;

  QString m_preferredAddress;
  QString m_preferredName;

  std::array<std::array<QIcon, 4>, 2> m_iconCache; // [MugState][PlateState]
};

#endif // TRAYCONTROLLER_H
