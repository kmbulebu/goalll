#include "UARTProtocol.h"
#include <stdlib.h>
#include <string.h>

UARTProtocol::UARTProtocol(Stream& stream, size_t bufSize)
  : uart(stream), lineBuf(nullptr), bufLen(bufSize), idx(0),
    homeCb(nullptr), awayCb(nullptr), resetShortCb(nullptr), resetLongCb(nullptr)
{
  lineBuf = (char*)malloc(bufLen);
  if (lineBuf) lineBuf[0] = '\0';
}

UARTProtocol::~UARTProtocol() {
  if (lineBuf) free(lineBuf);
}

void UARTProtocol::poll() {
  while (uart.available()) {
    int c = uart.read();
    if (c < 0) continue;
    // Append if space, else drop until newline
    if (idx + 1 < bufLen) {
      lineBuf[idx++] = (char)c;
      if (c == '\n') {
        // Trim trailing \r and \n
        size_t len = idx;
        if (len > 0 && lineBuf[len-1] == '\n') len--;
        if (len > 0 && lineBuf[len-1] == '\r') len--;
        lineBuf[len] = '\0';
        processLine(lineBuf, len);
        idx = 0;
      }
    } else {
      // overflow: drop until newline
      if (c == '\n') idx = 0;
    }
  }
}

uint8_t UARTProtocol::calcLRCInclusive(const char* data, size_t len) {
  uint8_t sum = 0;
  for (size_t i = 0; i < len; ++i) sum = (uint8_t)(sum + (uint8_t)data[i]);
  return sum;
}

bool UARTProtocol::parseHexByte(const char* s, uint8_t &out) {
  auto hexVal = [](char ch)->int {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'A' && ch <= 'F') return 10 + (ch - 'A');
    if (ch >= 'a' && ch <= 'f') return 10 + (ch - 'a');
    return -1;
  };
  int hi = hexVal(s[0]);
  int lo = hexVal(s[1]);
  if (hi < 0 || lo < 0) return false;
  out = (uint8_t)((hi << 4) | lo);
  return true;
}

void UARTProtocol::processLine(const char* line, size_t len) {
  // Ignore empty lines
  if (len == 0) return;
  // Must start with '^' otherwise ignore
  if (line[0] != '^') return;

  const char* star = strchr(line, '*');
  if (!star) return;
  size_t starPos = star - line;

  // Need two hex chars after '*'
  if (starPos + 3 > len) return;

  uint8_t recvCs;
  if (!parseHexByte(&line[starPos + 1], recvCs)) return;

  // Sum bytes from '^' (index 0) through '*' inclusive
  uint8_t lrc = calcLRCInclusive(line, starPos + 1);
  if (lrc != recvCs) return;

  // Extract payload between '^' and '*' (exclusive)
  size_t payloadLen = (starPos > 0) ? (starPos - 1) : 0;
  if (payloadLen == 0) return;

  // copy to temp buffer
  static char payload[128];
  if (payloadLen >= sizeof(payload)) payloadLen = sizeof(payload) - 1;
  memcpy(payload, &line[1], payloadLen);
  payload[payloadLen] = '\0';

  // Tokenize
  char *saveptr = nullptr;
  char *tok = strtok_r(payload, " ", &saveptr);
  if (!tok) return;

  if (strcmp(tok, "HG") == 0 || strcmp(tok, "AG") == 0) {
    char* tkn = strtok_r(nullptr, " ", &saveptr);
    if (!tkn) return;
    unsigned long t = strtoul(tkn, nullptr, 10);
    if (strcmp(tok, "HG") == 0) {
      if (homeCb) homeCb(t);
    } else {
      if (awayCb) awayCb(t);
    }
    return;
  }

  if (strcmp(tok, "RS") == 0) {
    if (resetShortCb) resetShortCb();
    return;
  }

  if (strcmp(tok, "RL") == 0) {
    if (resetLongCb) resetLongCb();
    return;
  }

  // Unknown type: ignore
}
