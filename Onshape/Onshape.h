#ifndef Onshape_h
#define Onshape_h

#include "Arduino.h"

#include <SPI.h>

#include <WiFiNINA.h>

class Onshape {
public:
    Onshape(const char accessKey[],
        const char secretKey[]);
    void start(const char ssid[]);
    void start(const char ssid[],
        const char pass[]);

    char* makeRequest(const char reqType[],
        const char path[]);
    char* makeRequest(const char reqType[],
        const char path[],
        const char data[]);
    WiFiSSLClient makeRequestStream(const char reqType[],
        const char path[]);
    WiFiSSLClient makeRequestStream(const char reqType[],
        const char path[],
        const char data[]);
    void handleChunkRequest(const char reqType[],
        const char path[], char* outputString);

private:
    const char* _accessKey;
    const char* _secretKey;
    WiFiSSLClient _client;
    char serverResponse[5000];

    void connectWifi(const char ssid[]);
    void connectWifi(const char ssid[],
        const char pass[]);

    void printWifiStatus();

    void getHmac64(const char payload[],
        const char key[], char* out);
    void buildHeaders(const char method[],
        const char url[],
        const char contentType[], char* out);
};

#endif
