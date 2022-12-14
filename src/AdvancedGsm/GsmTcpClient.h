#ifndef Advanced_GsmTcpClient_h
#define Advanced_GsmTcpClient_h

#include <Client.h>
#include "../api/GsmModem.h"
#include "GsmLog.h"

class GsmTcpClient : public Client {
 public:
  // GsmTcpClient(GsmModem& modem);
  // GsmModem& getModem();

  // Client.h implementation
 public:
  virtual int connect(IPAddress ip, uint16_t port);
  virtual int connect(const char* host, uint16_t port);
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t* buf, size_t size);
  virtual int available();
  virtual int read();
  virtual int read(uint8_t* buf, size_t size);
  virtual int peek();
  virtual void flush();
  virtual void stop();
  virtual uint8_t connected();
  virtual operator bool();

  //  protected:
  //   GsmModem& modem;
};

#endif