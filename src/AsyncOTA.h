#ifndef AsyncOTA_h
#define AsyncOTA_h

// #warning AsyncOTA.loop(); is deprecated, please remove it from loop() if defined. This function will be removed in a future release.

#include "Arduino.h"
#include "stdlib_noniso.h"

#if defined(ESP8266)
#include "ESP8266WiFi.h"
#include "ESPAsyncTCP.h"
#include "flash_hal.h"
#include "FS.h"
#elif defined(ESP32)
#include "WiFi.h"
#include "AsyncTCP.h"
#include "Update.h"
#include "esp_int_wdt.h"
#include "esp_task_wdt.h"
#endif

#include "Hash.h"
#include "ESPAsyncWebServer.h"
#include "FS.h"

#ifndef FIRMWARE_VERSION_MAJOR
#define FIRMWARE_VERSION_MAJOR 1
#define FIRMWARE_VERSION_MINOR 0
#define FIRMWARE_VERSION_PATCH 0
#endif

#include "Webpage.h"

class AsyncOtaClass
{

public:
    void
    setID(const char *id),
        begin(AsyncWebServer *server, const char *username = "", const char *password = ""),
        loop(),
        restart();

private:
    AsyncWebServer *_server;

    String getID();

    String _id = getID();
    String _username = "";
    String _password = "";
    bool _authRequired = false;
};

extern AsyncOtaClass AsyncOTA;

#endif
