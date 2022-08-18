#ifndef Advanced_GsmHttpClient_h
#define Advanced_GsmHttpClient_h

#include "../api/HttpClient.h"
#include "GsmLog.h"
#include "GsmModemCommon.h"
#include "GsmTcpClient.h"

enum UrlScheme {
  SCHEME_UNKNOWN = 0,
  SCHEME_HTTP = 1,
  SCHEME_HTTPS = 2,
};

// const char* GSM_PREFIX_HTTP = "http://";
// const int8_t GSM_PREFIX_HTTP_LENGTH = 7;
// const char* GSM_PREFIX_HTTPS = "https://";
// const int8_t GSM_PREFIX_HTTPS_LENGTH = 8;

#define GSM_PREFIX_HTTP "http://"
#define GSM_PREFIX_HTTP_LENGTH 7
#define GSM_PREFIX_HTTPS "https://"
#define GSM_PREFIX_HTTPS_LENGTH 8

#define GSM_HTTP_HEADER_BUFFER 500
#define GSM_HTTP_BODY_BUFFER 2000
#define GSM_HTTP_RESPONSE_WAIT 500
#define GSM_HTTP_RESPONSE_TIMEOUT_DEFAULT 30000

#define GSM_HTTP_ERROR_TIMED_OUT -1

class GsmHttpClient : public HttpClient {
 public:
  GsmHttpClient(GsmTcpClient& aClient,
                const char* aServerName,
                uint16_t aServerPort = HttpPort);
  // GsmHttpClient(GsmTcpClient& aClient, const String& aServerName, uint16_t
  // aServerPort = HttpPort); HttpClient(Client& aClient, const IPAddress&
  // aServerAddress, uint16_t aServerPort = kHttpPort);

  int get(const char* aURLPath);
  int get(const String& aURLPath);
  int post(const char* aURLPath);
  int post(const String& aURLPath);
  int post(const char* aURLPath, const char* aContentType, const char* aBody);
  int post(const String& aURLPath,
           const String& aContentType,
           const String& aBody);
  int post(const char* aURLPath,
           const char* aContentType,
           int aContentLength,
           const byte aBody[]);
  int put(const char* aURLPath);
  int put(const String& aURLPath);
  int put(const char* aURLPath, const char* aContentType, const char* aBody);
  int put(const String& aURLPath,
          const String& aContentType,
          const String& aBody);
  int put(const char* aURLPath,
          const char* aContentType,
          int aContentLength,
          const byte aBody[]);
  int patch(const char* aURLPath);
  int patch(const String& aURLPath);
  int patch(const char* aURLPath, const char* aContentType, const char* aBody);
  int patch(const String& aURLPath,
            const String& aContentType,
            const String& aBody);
  int patch(const char* aURLPath,
            const char* aContentType,
            int aContentLength,
            const byte aBody[]);
  int del(const char* aURLPath);
  int del(const String& aURLPath);
  int del(const char* aURLPath, const char* aContentType, const char* aBody);
  int del(const String& aURLPath,
          const String& aContentType,
          const String& aBody);
  int del(const char* aURLPath,
          const char* aContentType,
          int aContentLength,
          const byte aBody[]);

  // ================================================================

  virtual GsmModemCommon& getModem() = 0;

  // From HttpClient

  virtual bool completed();
  virtual int contentLength();
  //  virtual bool headerAvailable();
  virtual String responseBody();
  virtual int responseStatusCode();
  virtual int startRequest(const char* aURLPath,
                           const char* aHttpMethod,
                           const char* aContentType = NULL,
                           int aContentLength = -1,
                           const byte aBody[] = NULL) = 0;

  // From Client

  virtual void stop() = 0;
  virtual uint8_t connected() { return client->connected(); };
  virtual operator bool() { return bool(client); };
  virtual uint32_t httpResponseTimeout() { return http_response_timeout; };
  virtual void setHttpResponseTimeout(uint32_t timeout) {
    http_response_timeout = timeout;
  };

 protected:
  /** Reset internal state data back to the "just initialised" state
   */
  virtual void resetState();

  char body[GSM_HTTP_BODY_BUFFER] = {0};
  bool body_completed = false;
  GsmTcpClient* client;
  // bool iConnectionClose;
  int content_length;
  char headers[GSM_HTTP_HEADER_BUFFER] = {0};
  uint32_t http_response_timeout = GSM_HTTP_RESPONSE_TIMEOUT_DEFAULT;
  int response_status_code = 0;
  UrlScheme scheme;
  // IPAddress iServerAddress;
  const char* server_name;
  uint16_t server_port;
};

#endif
