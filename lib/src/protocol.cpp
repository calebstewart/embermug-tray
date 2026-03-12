#include "protocol.h"
#include "constants.h"

namespace Ember {

QByteArray Protocol::buildCommand(quint8 command, const QByteArray &payload) {
  QByteArray data;

  // Add sync bytes
  data.append(static_cast<char>(Constants::SYNC_BYTE_1));
  data.append(static_cast<char>(Constants::SYNC_BYTE_2));

  // Add command byte
  data.append(static_cast<char>(command));

  // Add payload length (2 bytes, little-endian)
  quint16 length = static_cast<quint16>(payload.size());
  data.append(static_cast<char>(length & 0xFF));
  data.append(static_cast<char>((length >> 8) & 0xFF));

  // Add payload
  data.append(payload);

  // Add checksum
  data.append(static_cast<char>(calculateChecksum(data)));

  return data;
}

quint8 Protocol::calculateChecksum(const QByteArray &data) {
  quint8 checksum = 0;
  for (int i = 0; i < data.size(); ++i) {
    checksum ^= static_cast<quint8>(data[i]);
  }
  return checksum;
}

QByteArray Protocol::encodeString(const QString &str) {
  QByteArray encoded;
  encoded.append(static_cast<char>(str.length()));

  QByteArray utf8 = str.toUtf8();
  encoded.append(utf8.constData(), utf8.size());

  return encoded;
}

QByteArray Protocol::ping() {
  return buildCommand(Constants::COMMAND_PING, QByteArray());
}

QByteArray Protocol::setTemperature(float celsius) {
  QByteArray payload;
  payload.append(static_cast<char>(TempUnit::Celsius));

  // Convert to integer * 100 for precision
  int tempValue = static_cast<int>(celsius * 100);
  payload.append(static_cast<char>(tempValue & 0xFF));
  payload.append(static_cast<char>((tempValue >> 8) & 0xFF));

  return buildCommand(Constants::COMMAND_SET_TARGET_TEMP, payload);
}

QByteArray Protocol::setTempUnit(TempUnit unit) {
  QByteArray payload;
  payload.append(static_cast<char>(unit));
  return buildCommand(Constants::COMMAND_SET_TEMP_UNIT, payload);
}

QByteArray Protocol::setMugName(const QString &name) {
  QByteArray payload = encodeString(name);
  return buildCommand(Constants::COMMAND_SET_MUG_NAME, payload);
}

QByteArray Protocol::setMugColor(quint8 red, quint8 green, quint8 blue) {
  QByteArray payload;
  payload.append(static_cast<char>(red));
  payload.append(static_cast<char>(green));
  payload.append(static_cast<char>(blue));
  return buildCommand(Constants::COMMAND_SET_MUG_COLOR, payload);
}

QByteArray Protocol::getMugState() {
  return buildCommand(Constants::COMMAND_GET_MUG_STATE, QByteArray());
}

Protocol::ResponseData Protocol::parseResponse(const QByteArray &data) {
  ResponseData result;
  result.type = ResponseType::Data;

  if (data.size() < 4) {
    result.type = ResponseType::Nack;
    result.error = QStringLiteral("Response too short");
    return result;
  }

  // Check sync bytes
  if (static_cast<quint8>(data[0]) != Constants::SYNC_BYTE_1 ||
      static_cast<quint8>(data[1]) != Constants::SYNC_BYTE_2) {
    result.type = ResponseType::Nack;
    result.error = QStringLiteral("Invalid sync bytes");
    return result;
  }

  // Get command/response byte
  quint8 commandByte = static_cast<quint8>(data[2]);

  // Determine response type
  if (commandByte == Constants::RESPONSE_ACK) {
    result.type = ResponseType::Ack;
  } else if (commandByte == Constants::RESPONSE_NACK) {
    result.type = ResponseType::Nack;
  } else if (commandByte == Constants::RESPONSE_DATA) {
    result.type = ResponseType::Data;
  }

  // Check checksum
  quint8 calculatedChecksum = calculateChecksum(data.left(data.size() - 1));
  quint8 receivedChecksum = static_cast<quint8>(data[data.size() - 1]);

  if (calculatedChecksum != receivedChecksum) {
    result.error = QStringLiteral("Checksum mismatch: expected 0x%1, got 0x%2")
                       .arg(calculatedChecksum, 2, 16, QChar('0'))
                       .arg(receivedChecksum, 2, 16, QChar('0'));
  }

  // Extract payload (skip header: 2 sync + 1 cmd + 2 length = 5 bytes, and 1
  // checksum)
  if (data.size() > 6) {
    result.payload = data.mid(5, data.size() - 6);
  }

  return result;
}

} // namespace Ember
