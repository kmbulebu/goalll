#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H

#include <Arduino.h>
#include <functional>

class UARTProtocol {
public:
  using GoalCb = std::function<void(unsigned long)>;
  using VoidCb = std::function<void(void)>;

  // Parser uses a pre-initialized Stream (e.g. Serial). The stream must be
  // initialized by the application (Serial.begin(...)) before calling poll().
  UARTProtocol(Stream& stream, size_t bufSize = 128);
  ~UARTProtocol();
  // Call frequently from loop()
  void poll();

  void setHomeGoalCallback(GoalCb cb) { homeCb = std::move(cb); }
  void setAwayGoalCallback(GoalCb cb) { awayCb = std::move(cb); }
  void setResetShortCallback(VoidCb cb) { resetShortCb = std::move(cb); }
  void setResetLongCallback(VoidCb cb) { resetLongCb = std::move(cb); }

private:
  Stream& uart;
  char *lineBuf;
  size_t bufLen;
  size_t idx;

  GoalCb homeCb;
  GoalCb awayCb;
  VoidCb resetShortCb;
  VoidCb resetLongCb;

  void processLine(const char* line, size_t len);
  static bool parseHexByte(const char* s, uint8_t &out);
  static uint8_t calcLRCInclusive(const char* data, size_t len);
};

#endif // UART_PROTOCOL_H
