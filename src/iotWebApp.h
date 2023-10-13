#ifndef IOTWEBAPP_H
#define IOTWEBAPP_H

#ifndef MAX_CHUNK_SIZE
#define MAX_CHUNK_SIZE 256
#endif

// Wipe webpage from memory
#include <Arduino.h>

#define ARDUINOJSON_USE_LONG_LONG 1

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#ifndef AsyncOTA_h
#include <AsyncOTA.h>
#endif
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include "index.h" // Include the webpage "index.h" header

typedef void (*FuncType)(uint8_t *, size_t);

class IotWebApp
{
public:
  IotWebApp();
  void init(const char *hostname, const char *webSocketPath);
  void loop(void);
  void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
  void sendBinaryOverWebSocket(const char *buffer, size_t bufferSize);
  void sendBinaryOverWebSocket(JsonDocument &jsonDoc);
  void sendJsonOverWebSocket(JsonDocument &jsonDoc);
  void onWebsocketData(FunctType callback);

private:
  AsyncWebServer server;
  AsyncWebSocket *ws;
  FunctType wsCallback;
};

#endif // IotWebApp_H
