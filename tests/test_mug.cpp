#include <QBluetoothDeviceInfo>
#include <QBluetoothUuid>
#include <QtTest>
#include <ember/mug.h>

class TestMug : public QObject {
  Q_OBJECT

private slots:
  void testServiceUuid();
  void testIsEmberMugByName();
  void testIsEmberMugByServiceUuid();
  void testIsEmberMugNegative();
};

void TestMug::testServiceUuid() {
  QBluetoothUuid uuid = Ember::Mug::serviceUuid();

  // Should return the known Ember Mug service UUID
  QCOMPARE(uuid.toString(QUuid::WithoutBraces).toLower(),
           QStringLiteral("fc543622-236c-4c94-8fa9-944a3e5353fa"));
}

void TestMug::testIsEmberMugByName() {
  // Device with "Ember" in name should be detected
  QBluetoothDeviceInfo emberDevice(QBluetoothAddress("00:11:22:33:44:55"),
                                   "Ember Mug 2", 0);
  QVERIFY(Ember::Mug::isEmberMug(emberDevice));

  // Case insensitive
  QBluetoothDeviceInfo emberLower(QBluetoothAddress("00:11:22:33:44:56"),
                                  "ember ceramic mug", 0);
  QVERIFY(Ember::Mug::isEmberMug(emberLower));

  // Name starting with Ember
  QBluetoothDeviceInfo emberStart(QBluetoothAddress("00:11:22:33:44:57"),
                                  "EmberMug", 0);
  QVERIFY(Ember::Mug::isEmberMug(emberStart));
}

void TestMug::testIsEmberMugByServiceUuid() {
  // Device advertising the Ember service UUID should be detected
  QBluetoothDeviceInfo deviceWithService(QBluetoothAddress("00:11:22:33:44:58"),
                                         "Unknown Device", 0);

  // Create a list with the Ember service UUID
  QList<QBluetoothUuid> services;
  services.append(Ember::Mug::serviceUuid());
  deviceWithService.setServiceUuids(services);

  QVERIFY(Ember::Mug::isEmberMug(deviceWithService));
}

void TestMug::testIsEmberMugNegative() {
  // Regular device should not be detected as Ember Mug
  QBluetoothDeviceInfo regularDevice(QBluetoothAddress("00:11:22:33:44:59"),
                                     "Some Other Device", 0);
  QVERIFY(!Ember::Mug::isEmberMug(regularDevice));

  // Device with similar but not matching name
  QBluetoothDeviceInfo notEmber(QBluetoothAddress("00:11:22:33:44:5A"),
                                "Remember Me Device", 0);
  QVERIFY(!Ember::Mug::isEmberMug(notEmber));

  // Empty name device
  QBluetoothDeviceInfo emptyName(QBluetoothAddress("00:11:22:33:44:5B"), "", 0);
  QVERIFY(!Ember::Mug::isEmberMug(emptyName));
}

QTEST_MAIN(TestMug)
#include "test_mug.moc"
