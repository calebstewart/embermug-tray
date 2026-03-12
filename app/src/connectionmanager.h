#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QObject>
#include <ember/mug.h>

class ConnectionManager : public QObject {
  Q_OBJECT

public:
  explicit ConnectionManager(QObject *parent = nullptr);
  ~ConnectionManager();

  void connectToDevice(const QBluetoothDeviceInfo &device);
  void disconnect();

  bool isConnected() const;
  bool isConnecting() const;
  Ember::Mug *mug() const;

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

private:
  void cleanup();

  QLowEnergyController *m_controller = nullptr;
  Ember::Mug *m_mug = nullptr;
  bool m_connecting = false;
};

#endif // CONNECTIONMANAGER_H
