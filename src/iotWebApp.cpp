#include "IotWebApp.h"
IotWebApp::IotWebApp() : server(80), ws(NULL)
{
}
void IotWebApp::init(const char *hostname, const char *webSocketPath)
{

  AsyncWiFiManager wifiManager(&server, NULL);
  wifiManager.autoConnect(hostname);

  if (MDNS.begin(hostname))
  {
    Serial.println("mDNS responder started");
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML, HTML_SIZE);
      response->addHeader("Content-Encoding", "gzip"); // Enable Gzip compression
      request->send(response); });

  ws = new AsyncWebSocket(webSocketPath);
  ws->onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
              { this->onWebSocketEvent(server, client, type, arg, data, len); });
  server.addHandler(ws);

  AsyncOTA.begin(&server, "esp-app", "esp8266!"); // Start AsyncElegantOTA

  server.begin();
}

void IotWebApp::onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  // WebSocket event handling code
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    // Need a Calback function to handle data
    Serial.printf("WebSocket client #%u data\n", client->id());
    break;
  case WS_EVT_PONG:
    Serial.printf("WebSocket client #%u pong\n", client->id());
    break;
  case WS_EVT_ERROR:
    Serial.printf("WebSocket client #%u error(%u): %s\n", client->id(), *((uint16_t *)arg), (char *)data);
    break;
  }
}

void IotWebApp::sendBinaryOverWebSocket(const char *buffer, size_t bufferSize)
{
  if (ws) // Check if the WebSocket instance is valid (not NULL)
    ws->binaryAll(buffer, bufferSize);
}

void IotWebApp::sendBinaryOverWebSocket(JsonDocument &jsonDoc)
{
  size_t msgPackSize = jsonDoc.memoryUsage() * 2;
  char msgPackBuffer[msgPackSize];
  size_t serializedSize = serializeMsgPack(jsonDoc, msgPackBuffer, msgPackSize);
  if (ws) // Check if the WebSocket instance is valid (not NULL)
    ws->binaryAll(msgPackBuffer, serializedSize);
}

void IotWebApp::sendJsonOverWebSocket(JsonDocument &jsonDoc)
{
  size_t jsonSize = jsonDoc.memoryUsage() * 2;
  char jsonBuffer[jsonSize];
  serializeJson(jsonDoc, jsonBuffer, jsonSize);
  if (ws) // Check if the WebSocket instance is valid (not NULL)
    ws->textAll(String(jsonBuffer));
}
