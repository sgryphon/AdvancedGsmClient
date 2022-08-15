#include "GsmModemCommon.h"

#include <Arduino.h>

// Public

GsmModemCommon::GsmModemCommon(Stream& stream) : stream(stream) {}

void GsmModemCommon::begin(const char accessPointName[],
                           PacketDataProtocolType pdpType,
                           const char username[],
                           const char password[]) {
  if (!reset()) {
    return;
  }
  connect(accessPointName, pdpType, username, password);
}

String GsmModemCommon::ICCID() {
  return "";
}

String GsmModemCommon::IMEI() {
  this->sendAT(GF("+CGSN"));
  String response;
  if (waitResponse(1000, response) != 1) {
    return "unknown";
  }
  response.replace("\r\nOK\r\n", "");
  response.replace("\rOK\r", "");
  response.trim();
  return response;
}

String GsmModemCommon::IMSI() {
  this->sendAT(GF("+CIMI"));
  String response;
  if (waitResponse(1000, response) != 1) {
    return "unknown";
  }
  response.replace("\r\nOK\r\n", "");
  response.replace("\rOK\r", "");
  response.trim();
  return response;
}

void GsmModemCommon::loop() {
  Serial.print("GsmModemCommon::begin\n");
}

String GsmModemCommon::manufacturer() {
  this->sendAT(GF("+CGMI"));
  String response;
  if (waitResponse(1000, response) != 1) {
    return "unknown";
  }
  response.replace("\r\nOK\r\n", "");
  response.replace("\rOK\r", "");
  response.trim();
  return response;
}

String GsmModemCommon::model() {
  this->sendAT(GF("+CGMM"));
  String response;
  if (waitResponse(1000, response) != 1) {
    return "unknown";
  }
  response.replace("\r\nOK\r\n", "");
  response.replace("\rOK\r", "");
  response.trim();
  return response;
}

String GsmModemCommon::network() {
  // Gets the PLMN (Public Land Mobile Network) operator details
  this->sendAT(GF("+COPS?"));
  if (waitResponse("+COPS:") != 1) {
    return "";
  }
  String plmn_details = this->stream.readStringUntil('\n');
  waitResponse();
  int start = plmn_details.indexOf('"');
  if (start == -1) { return ""; }
  int end = plmn_details.indexOf('"', start + 1);
  if (end < start + 2) { return ""; }
  String network = plmn_details.substring(start + 1, end);
  // TODO: Could include the Access Technology, e.g. "(NB-S1)"
  return network;
}

String GsmModemCommon::readResponseLine() {
  return this->stream.readStringUntil('\n');
}

RegistrationStatus GsmModemCommon::registrationStatus() {
  // Registration status results are aligned across versions.
  // Override if needed:
  //  +CREG?
  //  +CGREP? (GPRS)
  //  +CEREG? (EPS)
  //  +C5GREG? (5G)
  this->sendAT(GF("+CEREG?"));
  if (waitResponse("+CEREG:") != 1) {
    return RegistrationStatus::Unknown;
  }
  streamSkipUntil(',');  // skip mode
  int16_t status = this->stream.parseInt();
  if (waitResponse() != 1) {
    return RegistrationStatus::Unknown;
  }
  return static_cast<RegistrationStatus>(status);
}

String GsmModemCommon::revision() {
  this->sendAT(GF("+CGMR"));
  String response;
  if (waitResponse(1000, response) != 1) {
    return "unknown";
  }
  response.replace("\r\nOK\r\n", "");
  response.replace("\rOK\r", "");
  response.trim();
  return response;
}

int32_t GsmModemCommon::RSSI() {
  this->sendAT(GF("+CSQ"));
  if (waitResponse("+CSQ:") != 1) {
    return 0;
  }
  int16_t rssi_index = streamGetIntBefore(',');
  if (waitResponse() != 1) {
    return 0;
  }
  if (rssi_index == 99) {
    return 0;
  }
  double rssidBm = -113.0 + (rssi_index * 2);
  return rssidBm;
}

void GsmModemCommon::sendATCommand(const char command[]) {
  streamWrite("AT", command, this->gsmNL);
  this->stream.flush();
}

// Protected

inline int16_t GsmModemCommon::streamGetIntBefore(char lastChar) {
  char buf[7];
  size_t bytesRead =
      this->stream.readBytesUntil(lastChar, buf, static_cast<size_t>(7));
  // if we read 7 or more bytes, it's an overflow
  if (bytesRead && bytesRead < 7) {
    buf[bytesRead] = '\0';
    int16_t res = atoi(buf);
    return res;
  }

  return -9999;
}

inline bool GsmModemCommon::streamSkipUntil(const char c,
                                            const uint32_t timeout_ms) {
  uint32_t startMillis = millis();
  while (millis() - startMillis < timeout_ms) {
    while (millis() - startMillis < timeout_ms && !this->stream.available()) {
      TINY_GSM_YIELD();
    }
    if (this->stream.read() == c) {
      return true;
    }
  }
  return false;
}

int8_t GsmModemCommon::waitResponse() {
  return waitResponse(GSM_OK);
}

int8_t GsmModemCommon::waitResponse(GsmConstStr r1,
                                    GsmConstStr r2,
                                    GsmConstStr r3,
                                    GsmConstStr r4,
                                    GsmConstStr r5) {
  return waitResponse(1000, r1, r2, r3, r4, r5);
}

int8_t GsmModemCommon::waitResponse(uint32_t timeout_ms,
                                    GsmConstStr r1,
                                    GsmConstStr r2,
                                    GsmConstStr r3,
                                    GsmConstStr r4,
                                    GsmConstStr r5) {
  String s;
  return waitResponse(timeout_ms, s, r1, r2, r3, r4, r5);
}

int8_t GsmModemCommon::waitResponse(uint32_t timeout_ms,
                                    String& data,
                                    GsmConstStr r1,
                                    GsmConstStr r2,
                                    GsmConstStr r3,
                                    GsmConstStr r4,
                                    GsmConstStr r5) {
  return waitResponseDevice(timeout_ms, data, r1, r2, r3, r4, r5);
}
