#include "constants.h"
#include "protocol.h"
#include <QtTest>

class TestProtocol : public QObject {
  Q_OBJECT

private slots:
  void testPingCommand();
  void testSetTemperatureCommand();
  void testSetTempUnitCommand();
  void testGetMugStateCommand();
  void testCommandStructure();
  void testParseAckResponse();
  void testParseNackResponse();
  void testParseDataResponse();
  void testParseInvalidResponse();
  void testChecksumCalculation();
};

void TestProtocol::testPingCommand() {
  QByteArray cmd = Ember::Protocol::ping();

  // Should have sync bytes, command, length, and checksum
  QVERIFY(cmd.size() >= 6);

  // Check sync bytes
  QCOMPARE(static_cast<quint8>(cmd[0]),
           static_cast<quint8>(Ember::Constants::SYNC_BYTE_1));
  QCOMPARE(static_cast<quint8>(cmd[1]),
           static_cast<quint8>(Ember::Constants::SYNC_BYTE_2));

  // Check command byte
  QCOMPARE(static_cast<quint8>(cmd[2]), Ember::Constants::COMMAND_PING);

  // Payload length should be 0 for ping
  quint16 length =
      static_cast<quint8>(cmd[3]) | (static_cast<quint8>(cmd[4]) << 8);
  QCOMPARE(length, static_cast<quint16>(0));
}

void TestProtocol::testSetTemperatureCommand() {
  float temp = 55.0f;
  QByteArray cmd = Ember::Protocol::setTemperature(temp);

  // Check command byte
  QCOMPARE(static_cast<quint8>(cmd[2]),
           Ember::Constants::COMMAND_SET_TARGET_TEMP);

  // Payload should have unit (1 byte) + temperature (2 bytes)
  quint16 length =
      static_cast<quint8>(cmd[3]) | (static_cast<quint8>(cmd[4]) << 8);
  QCOMPARE(length, static_cast<quint16>(3));

  // Check temperature encoding (55.0 * 100 = 5500 = 0x157C)
  // Payload starts at index 5
  quint8 unit = static_cast<quint8>(cmd[5]);
  QCOMPARE(unit, static_cast<quint8>(Ember::TempUnit::Celsius));

  int tempValue =
      static_cast<quint8>(cmd[6]) | (static_cast<quint8>(cmd[7]) << 8);
  QCOMPARE(tempValue, 5500);
}

void TestProtocol::testSetTempUnitCommand() {
  QByteArray cmdCelsius =
      Ember::Protocol::setTempUnit(Ember::TempUnit::Celsius);
  QByteArray cmdFahrenheit =
      Ember::Protocol::setTempUnit(Ember::TempUnit::Fahrenheit);

  // Check command byte
  QCOMPARE(static_cast<quint8>(cmdCelsius[2]),
           Ember::Constants::COMMAND_SET_TEMP_UNIT);
  QCOMPARE(static_cast<quint8>(cmdFahrenheit[2]),
           Ember::Constants::COMMAND_SET_TEMP_UNIT);

  // Check payload
  QCOMPARE(static_cast<quint8>(cmdCelsius[5]),
           static_cast<quint8>(Ember::TempUnit::Celsius));
  QCOMPARE(static_cast<quint8>(cmdFahrenheit[5]),
           static_cast<quint8>(Ember::TempUnit::Fahrenheit));
}

void TestProtocol::testGetMugStateCommand() {
  QByteArray cmd = Ember::Protocol::getMugState();

  // Check command byte
  QCOMPARE(static_cast<quint8>(cmd[2]),
           Ember::Constants::COMMAND_GET_MUG_STATE);

  // Payload length should be 0
  quint16 length =
      static_cast<quint8>(cmd[3]) | (static_cast<quint8>(cmd[4]) << 8);
  QCOMPARE(length, static_cast<quint16>(0));
}

void TestProtocol::testCommandStructure() {
  // All commands should follow the same structure:
  // [SYNC1][SYNC2][CMD][LEN_LO][LEN_HI][PAYLOAD...][CHECKSUM]

  QByteArray cmd = Ember::Protocol::ping();

  // Minimum size: 2 sync + 1 cmd + 2 len + 1 checksum = 6
  QVERIFY(cmd.size() >= 6);

  // Verify checksum (XOR of all bytes except checksum should equal checksum)
  quint8 calculated = 0;
  for (int i = 0; i < cmd.size() - 1; ++i) {
    calculated ^= static_cast<quint8>(cmd[i]);
  }
  QCOMPARE(calculated, static_cast<quint8>(cmd[cmd.size() - 1]));
}

void TestProtocol::testParseAckResponse() {
  // Build a mock ACK response
  QByteArray response;
  response.append(static_cast<char>(Ember::Constants::SYNC_BYTE_1));
  response.append(static_cast<char>(Ember::Constants::SYNC_BYTE_2));
  response.append(static_cast<char>(Ember::Constants::RESPONSE_ACK));
  response.append(static_cast<char>(0x00)); // length lo
  response.append(static_cast<char>(0x00)); // length hi

  // Calculate and append checksum
  quint8 checksum = 0;
  for (int i = 0; i < response.size(); ++i) {
    checksum ^= static_cast<quint8>(response[i]);
  }
  response.append(static_cast<char>(checksum));

  auto result = Ember::Protocol::parseResponse(response);
  QCOMPARE(result.type, Ember::Protocol::ResponseType::Ack);
  QVERIFY(result.error.isEmpty());
}

void TestProtocol::testParseNackResponse() {
  QByteArray response;
  response.append(static_cast<char>(Ember::Constants::SYNC_BYTE_1));
  response.append(static_cast<char>(Ember::Constants::SYNC_BYTE_2));
  response.append(static_cast<char>(Ember::Constants::RESPONSE_NACK));
  response.append(static_cast<char>(0x00));
  response.append(static_cast<char>(0x00));

  quint8 checksum = 0;
  for (int i = 0; i < response.size(); ++i) {
    checksum ^= static_cast<quint8>(response[i]);
  }
  response.append(static_cast<char>(checksum));

  auto result = Ember::Protocol::parseResponse(response);
  QCOMPARE(result.type, Ember::Protocol::ResponseType::Nack);
}

void TestProtocol::testParseDataResponse() {
  // Build a data response with temperature payload
  QByteArray response;
  response.append(static_cast<char>(Ember::Constants::SYNC_BYTE_1));
  response.append(static_cast<char>(Ember::Constants::SYNC_BYTE_2));
  response.append(static_cast<char>(Ember::Constants::RESPONSE_DATA));
  response.append(static_cast<char>(0x04)); // length lo = 4 bytes
  response.append(static_cast<char>(0x00)); // length hi

  // Payload: current temp = 55.00C (5500 = 0x157C), target = 57.00C (5700 =
  // 0x1644)
  response.append(static_cast<char>(0x7C)); // current temp lo
  response.append(static_cast<char>(0x15)); // current temp hi
  response.append(static_cast<char>(0x44)); // target temp lo
  response.append(static_cast<char>(0x16)); // target temp hi

  quint8 checksum = 0;
  for (int i = 0; i < response.size(); ++i) {
    checksum ^= static_cast<quint8>(response[i]);
  }
  response.append(static_cast<char>(checksum));

  auto result = Ember::Protocol::parseResponse(response);
  QCOMPARE(result.type, Ember::Protocol::ResponseType::Data);
  QVERIFY(result.error.isEmpty());
  QCOMPARE(result.payload.size(), 4);

  // Verify payload content
  int currentTemp = static_cast<quint8>(result.payload[0]) |
                    (static_cast<quint8>(result.payload[1]) << 8);
  int targetTemp = static_cast<quint8>(result.payload[2]) |
                   (static_cast<quint8>(result.payload[3]) << 8);
  QCOMPARE(currentTemp, 5500);
  QCOMPARE(targetTemp, 5700);
}

void TestProtocol::testParseInvalidResponse() {
  // Too short
  QByteArray tooShort = "abc";
  auto result1 = Ember::Protocol::parseResponse(tooShort);
  QCOMPARE(result1.type, Ember::Protocol::ResponseType::Nack);
  QVERIFY(!result1.error.isEmpty());

  // Invalid sync bytes
  QByteArray badSync;
  badSync.append(static_cast<char>(0x00));
  badSync.append(static_cast<char>(0x00));
  badSync.append(static_cast<char>(0x01));
  badSync.append(static_cast<char>(0x00));
  auto result2 = Ember::Protocol::parseResponse(badSync);
  QCOMPARE(result2.type, Ember::Protocol::ResponseType::Nack);
  QVERIFY(result2.error.contains("sync"));
}

void TestProtocol::testChecksumCalculation() {
  // Build a command and verify the checksum is correct
  QByteArray cmd = Ember::Protocol::setTemperature(50.0f);

  // Recalculate checksum
  quint8 calculated = 0;
  for (int i = 0; i < cmd.size() - 1; ++i) {
    calculated ^= static_cast<quint8>(cmd[i]);
  }

  QCOMPARE(calculated, static_cast<quint8>(cmd[cmd.size() - 1]));
}

QTEST_MAIN(TestProtocol)
#include "test_protocol.moc"
