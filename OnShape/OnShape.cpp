#include "OnShape.h"

#include <base64.hpp>

int status = WL_IDLE_STATUS;
const PROGMEM char server[] = "cad.onshape.com";

OnShape::OnShape(const char accessKey[], const char secretKey[]) {
  _accessKey = accessKey;
  _secretKey = secretKey;
}

void OnShape::start(const char ssid[], const char pass[]) {
  while (!Serial) {
    Serial.begin(9600);
  }

  randomSeed(analogRead(0));

  // connect to WiFi
  connectWifi(ssid, pass);

  _client.connect(server, 443);
  Serial.println(F("Connected to OnShape server"));
}

void OnShape::connectWifi(const char ssid[], const char pass[]) {
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

void OnShape::printWifiStatus() {
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

// For authentication signatures
void OnShape::buildNonce (char* out) {
  const char c_opts[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  char nonce[25];
  
  for (int i=0; i < 25; i++)
    nonce[i] = c_opts[ random(sizeof(c_opts)/sizeof(char) - 1) ];

  strcpy(out, nonce);
}

void OnShape::utcString (char* out) {
  unsigned long t = WiFi.getTime();
    
  char dayofweek[4];
  char monthstr[4];
  int date = day(t);

  strncpy(dayofweek, &"SunMonTueWedThuFriSat"[ (weekday(t)-1)*3 ], 3);
  strncpy(monthstr, &"JanFebMarAprMayJunJulAugSepOctNovDec"[ (month(t)-1)*3 ], 3);
  dayofweek[3] = '\0';
  monthstr[3] = '\0';
  
  char timestamp[9];
  snprintf_P(timestamp, sizeof(timestamp), PSTR("%02d:%02d:%02d"), hour(t), minute(t), second(t));
  
  char utc[30];
  snprintf_P(utc, sizeof(utc), PSTR("%s, %02d %s %d %s GMT"), dayofweek, date, monthstr, year(t), timestamp);

  strcpy(out, utc);
}

void OnShape::getHmac64 (const char payload[], const char key[], char* out) {
  Sha256 builder;

  unsigned char keybuf[ strlen(key) ];
  for (int i=0; i<strlen(key); i++)
    keybuf[i] = key[i];

  builder.initHmac((uint8_t*)keybuf, strlen(key));
  builder.print(payload);

  // Convert hmac to base64
  uint8_t* result = builder.resultHmac();
  unsigned char b64buf[ encode_base64_length(sizeof(result)) ];

  encode_base64((unsigned char *)result, 32, b64buf);

  strcpy(out, (char*)b64buf);
}

void OnShape::buildHeaders (const char method[], const char url[], const char nonce[], const char date[], const char contentType[], char* out) {
  char *temp = new char[strlen(url)+1];
  strcpy(temp, url);

  char *path = strtok(temp, "?");
  char *queries = strtok(nullptr, "?");

  delete[] temp;

  char payload[250];
  // big string, so we'll use flash memory instead of SRAM
  snprintf_P(payload, sizeof(payload), PSTR("%s\n%s\n%s\n%s\n%s\n%s\n"), method, nonce, date, contentType, path, queries);
  // convert to lowercase
  for (int i=0; payload[i]; i++)
    payload[i] = tolower(payload[i]);  

  char hmac[45];
  getHmac64(payload, _secretKey, hmac);

  char auth[100];
  snprintf_P(auth, sizeof(auth), PSTR("On %s:HmacSHA256:%s"), _accessKey, hmac);

  strcpy(out, auth);
}

// TODO: add connect before every request
String OnShape::makeRequest(const char reqType[], const char path[]) {
  if (_client.connected()) {
      char nonce[25];
      buildNonce(nonce);

      char utc[30];
      utcString(utc);

      char signature[100];
      buildHeaders( reqType, path, nonce, utc, "application/json", signature );
      
      // Make a HTTP request:
      _client.print(reqType);
      _client.print(F(" "));
      _client.print(path);
      _client.println(F(" HTTP/1.1"));

      _client.println(F("Host: cad.onshape.com"));
      _client.println(F("Connection: keep-alive"));
      _client.println(F("Keep-Alive: timeout=5, max=100"));
      
      _client.println(F("Content-Type: application/json"));
      
      _client.print(F("Date: "));
      _client.println(utc);

      _client.print(F("On-Nonce: "));
      _client.println(nonce);

      _client.print(F("Authorization: "));
      _client.println(signature);
      
      _client.println(F("Accept: application/vnd.onshape.v1+json"));
  
      _client.println();
  
      int i=0;
      while (!_client.available() && i<1000) {
        delay(10);
        i++;
      }
      
      // Skip headers (we just want JSON at the moment)
      char endOfHeaders[] = "\r\n\r\n";
      _client.find(endOfHeaders);
  
      // char response[10];
      String response;
      char input;

      int j=0;
      do {
        input = _client.read();
        response += input;
        // snprintf_P(response+(j++), 2, PSTR("%c"), input);
      } while(_client.available());

      return response;
  } else {
    _client.connect(server, 443);
    return PSTR("no connection");
  }
}

/* POST DATA */
String OnShape::makeRequest(const char reqType[], const char path[], const char data[]) {
  if (_client.connected()) {
    char nonce[25];
    buildNonce(nonce);

    char utc[30];
    utcString(utc);

    char signature[100];
    buildHeaders( reqType, path, nonce, utc, "application/json", signature );
    
    // Make a HTTP request:
    _client.print(reqType);
    _client.print(F(" "));
    _client.print(path);
    _client.println(F(" HTTP/1.1"));

    _client.println(F("Host: cad.onshape.com"));
    _client.println(F("Connection: keep-alive"));
    _client.println(F("Keep-Alive: timeout=5, max=100"));
    _client.println(F("Accept: application/vnd.onshape.v1+json;charset=UTF-8;qs=0.1"));
    
    _client.println(F("Content-Type: application/json"));

    _client.print(F("Content-Length: "));
    _client.println(strlen(data));

    _client.print(F("Date: "));
    _client.println(utc);

    _client.print(F("On-Nonce: "));
    _client.println(nonce);

    _client.print(F("Authorization: "));
    _client.println(signature);

    _client.println(F("Accept: application/vnd.onshape.v1+json"));  
    _client.println();
    _client.println(data);


    int i=0;
    while (!_client.available() && i<1000) {
      delay(10);
      i++;
    }
    
    // Skip headers (we just want JSON at the moment)
    char endOfHeaders[] = "\r\n\r\n";
    _client.find(endOfHeaders);

    String response;
    char input;

    do {
      input = _client.read();
      response += input;
    } while(_client.available());

    if (response.length() == 1) {
      _client.stop();
      _client.connect(server, 443);
      makeRequest(reqType, path, data);
    }

    return response;
  } else {
      _client.connect(server, 443);
      return PSTR("no connection");
  }
}

/* Request stream functions */
/* Sometimes the response JSON will be too big to be stored in memory (i.e. for GET /api/documents/) */
/* In such cases, use makeRequestStream to get back a stream of the data, which can you can then handle as you wish */ 
WiFiSSLClient OnShape::makeRequestStream(const char reqType[], const char path[]) {
  if (_client.connected()) {
      char nonce[25];
      buildNonce(nonce);

      char utc[30];
      utcString(utc);

      char signature[100];
      buildHeaders( reqType, path, nonce, utc, "application/json", signature );
      
      // Make a HTTP request:
      _client.print(reqType);
      _client.print(F(" "));
      _client.print(path);
      _client.println(F(" HTTP/1.1"));

      _client.println(F("Host: cad.onshape.com"));
      _client.println(F("Connection: keep-alive"));
      _client.println(F("Keep-Alive: timeout=5, max=100"));
      
      _client.println(F("Content-Type: application/json"));
      
      _client.print(F("Date: "));
      _client.println(utc);

      _client.print(F("On-Nonce: "));
      _client.println(nonce);

      _client.print(F("Authorization: "));
      _client.println(signature);
      
      _client.println(F("Accept: application/vnd.onshape.v1+json"));
  
      _client.println();
  
      int i=0;
      while (!_client.available() && i<1000) {
        delay(10);
        i++;
      }
      
      // Skip headers (we just want JSON at the moment)
      char endOfHeaders[] = "\r\n\r\n";
      _client.find(endOfHeaders);
  
      return _client;
  } else {
    _client.connect(server, 443);
    return NULL;
  }
}

/* POST DATA */
WiFiSSLClient OnShape::makeRequestStream(const char reqType[], const char path[], const char data[]) {
  if (_client.connected()) {
    char nonce[25];
    buildNonce(nonce);

    char utc[30];
    utcString(utc);

    char signature[100];
    buildHeaders( reqType, path, nonce, utc, "application/json", signature );
    
    // Make a HTTP request:
    _client.print(reqType);
    _client.print(F(" "));
    _client.print(path);
    _client.println(F(" HTTP/1.1"));

    _client.println(F("Host: cad.onshape.com"));
    _client.println(F("Connection: keep-alive"));
    _client.println(F("Keep-Alive: timeout=5, max=100"));
    _client.println(F("Accept: application/vnd.onshape.v1+json;charset=UTF-8;qs=0.1"));
    
    _client.println(F("Content-Type: application/json"));

    _client.print(F("Content-Length: "));
    _client.println(strlen(data));

    _client.print(F("Date: "));
    _client.println(utc);

    _client.print(F("On-Nonce: "));
    _client.println(nonce);

    _client.print(F("Authorization: "));
    _client.println(signature);

    _client.println(F("Accept: application/vnd.onshape.v1+json"));  
    _client.println();
    _client.println(data);


    int i=0;
    while (!_client.available() && i<1000) {
      delay(10);
      i++;
    }
    
    // Skip headers (we just want JSON at the moment)
    char endOfHeaders[] = "\r\n\r\n";
    _client.find(endOfHeaders);

    return _client;
  } else {
      _client.connect(server, 443);
      return NULL;
  }
}

/* WRAPPER FUNCTIONS */

String OnShape::getDocument(const char documentID[]) {
  char url[50];
  strcpy(url, "/api/documents/");
  strcat(url, documentID);

  return makeRequest("GET", url);
}

String OnShape::newDocument(const char docName[]) {
  char payload[100] ="{\"name\": \"";
  strcat(payload,docName);
  char aa[]="\" ,\"ownerType\": \"0\",\"isPublic\": \"False\" }";
  strcat(payload,aa);

  return makeRequest("POST", "/api/documents", payload);
}