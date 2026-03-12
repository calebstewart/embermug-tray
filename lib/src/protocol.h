#ifndef EMBER_PROTOCOL_H
#define EMBER_PROTOCOL_H

#include <QByteArray>
#include <ember/types.h>

namespace Ember {

class Protocol {
public:
  // Build command packets
  static QByteArray ping();
  static QByteArray setTemperature(float celsius);
  static QByteArray setTempUnit(TempUnit unit);
  static QByteArray setMugName(const QString &name);
  static QByteArray setMugColor(quint8 red, quint8 green, quint8 blue);
  static QByteArray getMugState();

  // Response parsing
  enum class ResponseType { Ack, Nack, Data };

  struct ResponseData {
    ResponseType type;
    QByteArray payload;
    QString error;
  };

  static ResponseData parseResponse(const QByteArray &data);

private:
  static QByteArray buildCommand(quint8 command, const QByteArray &payload);
  static quint8 calculateChecksum(const QByteArray &data);
  static QByteArray encodeString(const QString &str);
};

} // namespace Ember

#endif // EMBER_PROTOCOL_H
