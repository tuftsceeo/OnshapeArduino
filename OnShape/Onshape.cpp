#include "Onshape.h"

#include "base64.hpp"


int status = WL_IDLE_STATUS;
const PROGMEM char server[] = "cad.onshape.com";

Onshape::Onshape(const char accessKey[],
  const char secretKey[]) {
  _accessKey = accessKey;
  _secretKey = secretKey;
}

void Onshape::start(const char ssid[]) {
  while (!Serial) {
    Serial.begin(9600);
  }

  // connect to WiFi
  connectWifi(ssid);

  while (!_client.connected()) {
    if (!_client.connectSSL(server, 443)) {
      _client.flush();
      _client.stop();
    }
    Serial.println("Trying to Connect to Onshape Server");
  };
  Serial.println(F("Connected to Onshape server"));
}

void Onshape::start(const char ssid[],
  const char pass[]) {
  while (!Serial) {
    Serial.begin(9600);
  }

  // connect to WiFi
  connectWifi(ssid, pass);

  while (!_client.connected()) {
    if (!_client.connectSSL(server, 443)) {
      _client.flush();
      _client.stop();
    }
    Serial.println("Trying to Connect to Onshape Server");
  };
  Serial.println(F("Connected to Onshape server"));

}

void Onshape::connectWifi(const char ssid[]) {
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println(F("WiFi shield not present"));
    // no shield -- SHUT IT DOWN AHHHH!
    while (true);
  }

  Serial.println(WiFi.firmwareVersion());

  while (status != WL_CONNECTED) {
    Serial.print(F("Attempting to connect to SSID: "));
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid);

    delay(10000);
  }
  Serial.println(F("Connected to wifi"));

  printWifiStatus();
}

void Onshape::connectWifi(const char ssid[],
  const char pass[]) {
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println(F("WiFi shield not present"));
    // no shield -- SHUT IT DOWN AHHHH!
    while (true);
  }

  Serial.println(WiFi.firmwareVersion());

  while (status != WL_CONNECTED) {
    Serial.print(F("Attempting to connect to SSID: "));
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);

    delay(10000);
  }
  Serial.println(F("Connected to wifi"));

  printWifiStatus();
}

void Onshape::printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print(F("IP Address: "));
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print(F("signal strength (RSSI):"));
  Serial.print(rssi);
  Serial.println(F(" dBm"));
}

void Onshape::buildHeaders(const char method[],
  const char url[],
    const char contentType[], char * out) {
  char * temp = new char[strlen(url) + 1];
  strcpy(temp, url);

  char * path = strtok(temp, "?");
  char * queries = strtok(nullptr, "?");

  delete[] temp;

  char temparr[100];
  const char * colonchr = ":";

  strcpy(temparr, _accessKey);
  strcat(temparr, colonchr);
  strcat(temparr, _secretKey);

  const char * rawData = temparr;
  size_t rawLength = strlen(temparr);

  char encoded[BASE64::encodeLength(rawLength)];
  BASE64::encode((const uint8_t * ) rawData, rawLength, encoded);

  strcpy(out, encoded);

}

// TODO: add connect before every request
char * Onshape::makeRequest(const char reqType[],
  const char path[]) {

  while (_client.connected()) {
    char signature[100];
    buildHeaders(reqType, path, "application/json", signature);
    // Make a HTTP request:
    _client.print(reqType);
    _client.print(F(" "));
    _client.print(path);
    _client.println(F(" HTTP/1.1"));

    _client.println(F("Host: cad.onshape.com"));
    _client.println(F("Connection: keep-alive"));
    //_client.println(F("Keep-Alive: timeout=100, max=100"));

    _client.println(F("Content-Type: application/vnd.onshape.v2+json;charset=UTF-8;qs=0.2"));

    _client.print(F("Authorization: Basic "));
    _client.println(F(signature));

    _client.println(F("Accept: application/vnd.onshape.v2+json;charset=UTF-8;qs=0.2"));

    _client.println();

    int i = 0;
    while (!_client.available() && i < 300) {
      delay(10);
      i++;
    }
    if (i == 300) {
      break;
    }

    // Skip headers (we just want JSON at the moment)
    char endOfHeaders[] = "\r\n\r\n";
    _client.find(endOfHeaders);

    char input;
    int count = 0;

    do {
      input = _client.read();
      if (input != ' ' && input != '\n') {
        serverResponse[count] = input;
        count++;
      }
    } while (_client.available());

    serverResponse[count] = '\0';

    return serverResponse;
  }
  _client.flush();
  _client.stop();
  Serial.println("Lost connection, reconnecting");
  if (_client.connect(server, 443)) {
    makeRequest(reqType, path);
  }

}

/* POST DATA */
char * Onshape::makeRequest(const char reqType[],
  const char path[],
    const char data[]) {
  while (_client.connected()) {
    char signature[100];
    buildHeaders(reqType, path, "application/vnd.onshape.v2+json;charset=UTF-8;qs=0.2", signature);

    // Make a HTTP request:
    _client.print(reqType);
    _client.print(F(" "));
    _client.print(path);
    _client.println(F(" HTTP/1.1"));

    _client.println(F("Host: cad.onshape.com"));
    _client.println(F("Connection: keep-alive"));
    //_client.println(F("Keep-Alive: timeout=100, max=100"));
    _client.println(F("Content-Type: application/vnd.onshape.v2+json;charset=UTF-8;qs=0.2"));
    _client.print(F("Content-Length: "));
    _client.println(strlen(data));
    _client.println(F("Accept: application/vnd.onshape.v2+json;charset=UTF-8;qs=0.2"));
    _client.print(F("Authorization: Basic "));
    _client.println(F(signature));
    _client.println();
    _client.println(F(data));

    int i = 0;
    while (!_client.available() && i < 300) {
      delay(10);
      i++;
    }
    if (i == 300) {
      break;
    }

    // Skip headers (we just want JSON at the moment)
    char endOfHeaders[] = "\r\n\r\n";
    _client.find(endOfHeaders);

    // char response[10];
    char input;
    int count = 0;

    do {
      input = _client.read();
      if (input != ' ' && input != '\n') {
        serverResponse[count] = input;
        count++;
      }
    } while (_client.available());

    serverResponse[count] = '\0';
    return serverResponse;
  }

  _client.flush();
  _client.stop();
  Serial.println("Lost connection, reconnecting");
  if (_client.connect(server, 443)) {
    makeRequest(reqType, path, data);
  }

}

WiFiSSLClient Onshape::makeRequestStream(const char reqType[],
  const char path[]) {
  while (_client.connected()) {
    char signature[100];
    buildHeaders(reqType, path, "application/json", signature);
    // Make a HTTP request:
    _client.print(reqType);
    _client.print(F(" "));
    _client.print(path);
    _client.println(F(" HTTP/1.1"));

    _client.println(F("Host: cad.onshape.com"));
    _client.println(F("Connection: keep-alive"));
    //_client.println(F("Keep-Alive: timeout=100, max=100"));

    _client.println(F("Content-Type: application/vnd.onshape.v2+json;charset=UTF-8;qs=0.2"));

    _client.print(F("Authorization: Basic "));
    _client.println(F(signature));

    _client.println(F("Accept: application/vnd.onshape.v2+json;charset=UTF-8;qs=0.2"));

    _client.println();

    int i = 0;
    while (!_client.available() && i < 300) {
      delay(10);
      i++;
    }
    if (i == 300) {
      break;
    }

    // Skip headers (we just want JSON at the moment)
    char endOfHeaders[] = "\r\n\r\n";
    _client.find(endOfHeaders);

    // char response[10];

    return _client;
  }
  _client.flush();
  _client.stop();
  Serial.println("Lost connection, reconnecting");
  if (_client.connect(server, 443)) {
    makeRequestStream(reqType, path);
  }

}

/* POST DATA */
WiFiSSLClient Onshape::makeRequestStream(const char reqType[],
  const char path[],
    const char data[]) {

  while (_client.connected()) {

    char signature[100];
    buildHeaders(reqType, path, "application/vnd.onshape.v2+json;charset=UTF-8;qs=0.2", signature);

    // Make a HTTP request:
    _client.print(reqType);
    _client.print(F(" "));
    _client.print(path);
    _client.println(F(" HTTP/1.1"));

    _client.println(F("Host: cad.onshape.com"));
    _client.println(F("Connection: keep-alive"));
    //_client.println(F("Keep-Alive: timeout=100, max=100"));
    _client.println(F("Content-Type: application/vnd.onshape.v2+json;charset=UTF-8;qs=0.2"));
    _client.print(F("Content-Length: "));
    _client.println(strlen(data));
    _client.println(F("Accept: application/vnd.onshape.v2+json;charset=UTF-8;qs=0.2"));
    _client.print(F("Authorization: Basic "));
    _client.println(F(signature));
    _client.println();
    _client.println(F(data));

    int i = 0;
    while (!_client.available() && i < 300) {
      delay(10);
      i++;
    }
    if (i == 300) {
      break;
    }

    char endOfHeaders[] = "\r\n\r\n";
    _client.find(endOfHeaders);

    return _client;

  }

  _client.flush();
  _client.stop();
  Serial.println("Lost connection, reconnecting");
  if (_client.connect(server, 443)) {
    makeRequestStream(reqType, path, data);
  }

}

void Onshape::handleChunkRequest(const char reqType[],
  const char path[], char * outputString) {
  while (_client.connected()) {
    char signature[100];

    buildHeaders(reqType, path, "application/json", signature);
    // Make a HTTP request:
    _client.print(reqType);
    _client.print(F(" "));
    _client.print(path);
    _client.println(F(" HTTP/1.1"));

    _client.println(F("Host: cad.onshape.com"));

    _client.println(F("Connection: keep-alive"));
    //_client.println(F("Keep-Alive: timeout=100, max=100"));

    _client.println(F("Content-Type: application/vnd.onshape.v2+json;charset=UTF-8;qs=0.2"));

    _client.print(F("Authorization: Basic "));
    _client.println(F(signature));

    _client.println(F("Accept: application/vnd.onshape.v2+json;charset=UTF-8;qs=0.2"));

    _client.println();

    int i = 0;
    while (!_client.available() && i < 300) {
      delay(10);
      i++;
    }
    if (i == 300) {
      break;
    }

    char endOfHeaders[] = "\r\n\r\n";
    _client.find(endOfHeaders);

    int lenTrav = 0;
    int strPtr = 0;
    char input = ' ';
    bool endFlag = false;

    int prevChunkLen = 0;

    char chunkLenString[] = "0000";

    i = 0;
    while (input != '\r' && _client.available()) {
      input = _client.read();
      chunkLenString[i] = input;
      i++;
    }
    chunkLenString[i] = '\0';

    int chunkLen = std::stoi(chunkLenString, 0, 16);

    while (!endFlag) {

      _client.read();

      while (lenTrav - prevChunkLen != chunkLen) {
        while (!_client.available()) {}
        input = _client.read();
        if (input != ' ' && input != '\n' && input != '\r') {
          outputString[strPtr] = input;
          strPtr++;
        }
        lenTrav++;
      }

      _client.read();
      _client.read();

      prevChunkLen += chunkLen;

      i = 0;
      while (input != '\r') {
        input = _client.read();
        chunkLenString[i] = input;
        i++;
      }
      chunkLenString[i] = '\0';

      chunkLen = std::stoi(chunkLenString, 0, 16);
      if (chunkLen == 0) {
        endFlag = true;
      }

    }

    outputString[strPtr] = '\0';
    return;

  }
  _client.flush();
  _client.stop();
  Serial.println("Lost connection, reconnecting");
  if (_client.connect(server, 443)) {
    handleChunkRequest(reqType, path, outputString);
  }

}
