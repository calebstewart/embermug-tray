#include <QApplication>
#include <QDebug>

#include "connectionmanager.h"
#include "devicemonitor.h"
#include "traycontroller.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setOrganizationName(QStringLiteral("embermug"));
  app.setApplicationName(QStringLiteral("tray"));
  app.setApplicationVersion(QStringLiteral("0.2.0"));
  app.setQuitOnLastWindowClosed(false);

  qInfo() << "embermug-tray started";

  DeviceMonitor monitor;
  ConnectionManager connectionManager;
  TrayController trayController(&monitor, &connectionManager);

  trayController.show();

  qInfo() << "System tray icon created";

  return app.exec();
}
