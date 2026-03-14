#include <QApplication>
#include <QDebug>

#include "connectionmanager.h"
#include "devicemonitor.h"
#include "traycontroller.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  QApplication::setOrganizationName(QStringLiteral("embermug"));
  QApplication::setApplicationName(QStringLiteral("tray"));
  QApplication::setApplicationVersion(QStringLiteral("0.2.0"));
  QApplication::setQuitOnLastWindowClosed(false);

  qInfo() << "embermug-tray started";

  DeviceMonitor monitor;
  ConnectionManager connectionManager;
  TrayController trayController(&monitor, &connectionManager);

  trayController.show();

  qInfo() << "System tray icon created";

  return QApplication::exec();
}
