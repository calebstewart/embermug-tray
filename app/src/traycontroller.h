#ifndef TRAYCONTROLLER_H
#define TRAYCONTROLLER_H

#include <QAction>
#include <QMenu>
#include <QObject>
#include <QSystemTrayIcon>
#include <QTimer>
#include <ember/ember.h>

class DeviceMonitor;
class ConnectionManager;

class TrayController : public QObject {
  Q_OBJECT

public:
  explicit TrayController(DeviceMonitor *monitor, ConnectionManager *manager,
                          QObject *parent = nullptr);
  ~TrayController();

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
  void updateMugDisplay();
  void updateTrayTooltip();

  DeviceMonitor *m_monitor;
  ConnectionManager *m_manager;

  QSystemTrayIcon *m_trayIcon;
  QMenu *m_contextMenu;
  QMenu *m_devicesMenu;
  QAction *m_statusAction;
  QAction *m_tempAction;
  QAction *m_batteryAction;
  QAction *m_scanAction;
  QAction *m_disconnectAction;
  QAction *m_quitAction;

  QTimer m_refreshTimer;
};

#endif // TRAYCONTROLLER_H
