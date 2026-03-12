#include <QtTest>
#include <ember/types.h>

class TestTypes : public QObject {
  Q_OBJECT

private slots:
  void testCelsiusToFahrenheit();
  void testFahrenheitToCelsius();
  void testTemperatureRoundTrip();
  void testTempUnitToString();
  void testBatteryStateToString();
  void testLiquidStateToString();
  void testMugEventToString();
};

void TestTypes::testCelsiusToFahrenheit() {
  // Freezing point
  QCOMPARE(Ember::celsiusToFahrenheit(0.0f), 32.0f);

  // Boiling point
  QCOMPARE(Ember::celsiusToFahrenheit(100.0f), 212.0f);

  // Room temperature (~20C = 68F)
  QVERIFY(qFuzzyCompare(Ember::celsiusToFahrenheit(20.0f), 68.0f));

  // Typical coffee temperature (~55C = 131F)
  QVERIFY(qFuzzyCompare(Ember::celsiusToFahrenheit(55.0f), 131.0f));
}

void TestTypes::testFahrenheitToCelsius() {
  // Freezing point
  QCOMPARE(Ember::fahrenheitToCelsius(32.0f), 0.0f);

  // Boiling point
  QCOMPARE(Ember::fahrenheitToCelsius(212.0f), 100.0f);

  // Room temperature
  QVERIFY(qFuzzyCompare(Ember::fahrenheitToCelsius(68.0f), 20.0f));
}

void TestTypes::testTemperatureRoundTrip() {
  // Converting C -> F -> C should give back the original value
  float original = 55.5f;
  float fahrenheit = Ember::celsiusToFahrenheit(original);
  float backToCelsius = Ember::fahrenheitToCelsius(fahrenheit);
  QVERIFY(qFuzzyCompare(original, backToCelsius));

  // And F -> C -> F
  float originalF = 135.0f;
  float celsius = Ember::fahrenheitToCelsius(originalF);
  float backToFahrenheit = Ember::celsiusToFahrenheit(celsius);
  QVERIFY(qFuzzyCompare(originalF, backToFahrenheit));
}

void TestTypes::testTempUnitToString() {
  QCOMPARE(Ember::tempUnitToString(Ember::TempUnit::Celsius),
           QStringLiteral("Celsius"));
  QCOMPARE(Ember::tempUnitToString(Ember::TempUnit::Fahrenheit),
           QStringLiteral("Fahrenheit"));
}

void TestTypes::testBatteryStateToString() {
  QCOMPARE(Ember::batteryStateToString(Ember::BatteryState::Unknown),
           QStringLiteral("Unknown"));
  QCOMPARE(Ember::batteryStateToString(Ember::BatteryState::Charging),
           QStringLiteral("Charging"));
  QCOMPARE(Ember::batteryStateToString(Ember::BatteryState::Discharging),
           QStringLiteral("Discharging"));
  QCOMPARE(Ember::batteryStateToString(Ember::BatteryState::Full),
           QStringLiteral("Full"));
  QCOMPARE(Ember::batteryStateToString(Ember::BatteryState::NotCharging),
           QStringLiteral("Not Charging"));
}

void TestTypes::testLiquidStateToString() {
  QCOMPARE(Ember::liquidStateToString(Ember::LiquidState::Unknown),
           QStringLiteral("Unknown"));
  QCOMPARE(Ember::liquidStateToString(Ember::LiquidState::Empty),
           QStringLiteral("Empty"));
  QCOMPARE(Ember::liquidStateToString(Ember::LiquidState::Half),
           QStringLiteral("Half"));
  QCOMPARE(Ember::liquidStateToString(Ember::LiquidState::Full),
           QStringLiteral("Full"));
}

void TestTypes::testMugEventToString() {
  QCOMPARE(Ember::mugEventToString(Ember::MugEvent::None),
           QStringLiteral("None"));
  QCOMPARE(Ember::mugEventToString(Ember::MugEvent::CupRemoved),
           QStringLiteral("Cup Removed"));
  QCOMPARE(Ember::mugEventToString(Ember::MugEvent::CupPlaced),
           QStringLiteral("Cup Placed"));
  QCOMPARE(Ember::mugEventToString(Ember::MugEvent::TempReached),
           QStringLiteral("Temperature Reached"));
  QCOMPARE(Ember::mugEventToString(Ember::MugEvent::BatteryLow),
           QStringLiteral("Battery Low"));
  QCOMPARE(Ember::mugEventToString(Ember::MugEvent::HeatingStarted),
           QStringLiteral("Heating Started"));
  QCOMPARE(Ember::mugEventToString(Ember::MugEvent::HeatingStopped),
           QStringLiteral("Heating Stopped"));
}

QTEST_MAIN(TestTypes)
#include "test_types.moc"
