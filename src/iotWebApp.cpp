#include "IotWebApp.h"

void configModeCallback(AsyncWiFiManager *myWiFiManager)
{
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  // if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  // entered config mode, make led toggle faster
  //  ticker.attach(0.2, tick);
}

IotWebApp::IotWebApp() : server(80), ws(NULL)
{
}
void IotWebApp::loop(void)
{
  MDNS.update();
}
void IotWebApp::init(const char *hostname, const char *webSocketPath)
{

  DNSServer dns;
  AsyncWiFiManager wifiManager(&server, &dns);
  // wifiManager.autoConnect(hostname);

  // set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  // fetches ssid and pass and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect(hostname))
  {
    Serial.println("failed to connect and hit timeout");
    // reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  Serial.println("Start MDNS responder");
  if (MDNS.begin(hostname))
  {
    Serial.println("mDNS responder started");
  }

  // serveProgMem("/", HTML, HTML_SIZE);

  ws = new AsyncWebSocket(webSocketPath);
  ws->onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
              { this->onWebSocketEvent(server, client, type, arg, data, len); });
  server.addHandler(ws);

  // Handle WebSocket handshake with CORS headers
  server.on("/ws", HTTP_OPTIONS, [](AsyncWebServerRequest *request)
            {
  AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "OK");
  response->addHeader("Access-Control-Allow-Origin", "*");
  response->addHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  response->addHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
  request->send(response); });

  AsyncOTA.begin(&server, "esp-app", "esp8266!");

  server.begin();
}

void IotWebApp::serveProgMem(const char *uri, const uint8_t *content, const uint32_t contentLength, const char *contentType)
{
  server.on(uri, HTTP_GET, [content, contentLength](AsyncWebServerRequest *request)
            {
      AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", content, contentLength);
      response->addHeader("Content-Encoding", "gzip"); // Enable Gzip compression
      request->send(response); });
}

void IotWebApp::onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  // WebSocket event handling code
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    if (wsConnectCallback)
      wsConnectCallback(data, len);
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    // Need a Calback function to handle data
    Serial.printf("WebSocket client #%u data\n", client->id());
    if (wsEventCallback)
      wsEventCallback(data, len);
    break;
  case WS_EVT_PONG:
    Serial.printf("WebSocket client #%u pong\n", client->id());
    break;
  case WS_EVT_ERROR:
    Serial.printf("WebSocket client #%u error(%u): %s\n", client->id(), *((uint16_t *)arg), (char *)data);
    break;
  }
}
void IotWebApp::onWebsocketConnect(FuncType callback)
{
  wsConnectCallback = callback;
}

void IotWebApp::onWebsocketData(FuncType callback)
{
  wsEventCallback = callback;
}

void IotWebApp::sendBinaryOverWebSocket(const char *buffer, size_t bufferSize)
{
  if (ws) // Check if the WebSocket instance is valid (not NULL)
    ws->binaryAll(buffer, bufferSize);
}

void IotWebApp::sendMsgPackOverWebSocket(JsonDocument &jsonDoc)
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

void IotWebApp::sendText(const char *buffer, size_t bufferSize)
{
  if (ws) // Check if the WebSocket instance is valid (not NULL)
    ws->textAll(buffer, bufferSize);
}