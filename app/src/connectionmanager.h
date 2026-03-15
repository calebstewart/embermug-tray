#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <QBluetoothDeviceInfo>
#include <QBluetoothLocalDevice>
#include <QLowEnergyController>
#include <QObject>
#include <QSettings>
#include <QTimer>
#include <ember/mug.h>

class ConnectionManager : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ConnectionManager)

public:
  explicit ConnectionManager(QObject *parent = nullptr);
  ~ConnectionManager() override;

  void connectToDevice(const QBluetoothDeviceInfo &device);
  void disconnect();

  [[nodiscard]] bool isConnected() const;
  [[nodiscard]] bool isConnecting() const;
  [[nodiscard]] Ember::Mug *mug() const;
  [[nodiscard]] QBluetoothDeviceInfo connectedDevice() const;

signals:
  void connecting();
  void connected();
  void disconnected();
  void mugReady();
  void connectionFailed(const QString &error);

private slots:
  void onControllerConnected();
  void onControllerDisconnected();
  void onControllerError(QLowEnergyController::Error error);
  void onMugReady();
  void onMugError(const QString &error);
  void onInitTimeout();
  void onPairingFinished(const QBluetoothAddress &address,
                         QBluetoothLocalDevice::Pairing pairing);

private: // NOLINT(readability-redundant-access-specifiers)
  static constexpr int INIT_TIMEOUT_MS = 10000;

  void cleanup();
  void proceedWithConnection();

  QLowEnergyController *m_controller = nullptr;
  Ember::Mug *m_mug = nullptr;
  QBluetoothLocalDevice m_localDevice;
  QTimer m_initTimer;
  QBluetoothDeviceInfo m_currentDevice;
  bool m_connecting = false;
};

#endif // CONNECTIONMANAGER_H
