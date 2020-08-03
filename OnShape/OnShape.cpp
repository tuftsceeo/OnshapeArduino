#include "OnShape.h"

#include <base64.hpp>

int status = WL_IDLE_STATUS;
char server[] = "cad.onshape.com";

OnShape::OnShape(String accessKey, const char secretKey[]) {
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
  	Serial.println("Connected to OnShape server");
}

void OnShape::connectWifi(const char ssid[], const char pass[]) {
	if (WiFi.status() == WL_NO_SHIELD) {
		Serial.println("WiFi shield not present");
		// no shield -- SHUT IT DOWN AHHHH!
		while (true);
	}

	Serial.println(WiFi.firmwareVersion());

	while (status != WL_CONNECTED) {
		Serial.print("Attempting to connect to SSID: ");
		Serial.println(ssid);
		// Connect to WPA/WPA2 network
		status = WiFi.begin(ssid, pass);
	
		delay(10000);
	}
	Serial.println("Connected to wifi");

	printWifiStatus();
}

void OnShape::printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

// For authentication signatures
String OnShape::buildNonce () {
  char c_opts[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  String nonce = "";
  
  for (int i=0; i < 25; i++)
    nonce += c_opts[ random(sizeof(c_opts)/sizeof(char) - 1) ];
  
  return nonce;
}

String OnShape::utcString () {
  unsigned long t = WiFi.getTime();
    
  char dayofweek[4];
  char monthstr[4];
  int date = day(t);

  strncpy(dayofweek, &"SunMonTueWedThuFriSat"[ (weekday(t)-1)*3 ], 3);
  dayofweek[3] = '\0';
  strncpy(monthstr, &"JanFebMarAprMayJunJulAugSepOctNovDec"[ (month(t)-1)*3 ], 3);
  monthstr[3] = '\0';
  
  int h = hour(t);
  int m = minute(t);
  int s = second(t);
  String timestamp = ( (h<10) ? "0"+String(h) : String(h) ) + ":" + ( (m<10) ? "0"+String(m) : String(m) )  + ":" + ( (s<10) ? "0"+String(s) : String(s) ) ;
  
  return String(dayofweek) + ", " + ( (date<10) ? "0"+String(date) : String(date) ) + " " + String(monthstr) + " " + year(t) + " " + timestamp + " GMT";
}

String OnShape::getHmac64 (String payload, String key) {
  Sha256 builder;

  unsigned char keybuf[ key.length() ];
  for (int i=0; i<key.length(); i++)
    keybuf[i] = key[i];

  builder.initHmac((uint8_t*)keybuf, key.length());
  builder.print(payload);

  // Convert hmac to base64
  uint8_t* result = builder.resultHmac();
  unsigned char b64buf[ encode_base64_length(sizeof(result)) ];

  encode_base64((unsigned char *)result, 32, b64buf);

  return String((char *)b64buf);
}

String OnShape::buildHeaders (String method, String url, String nonce, String date, String contentType) {
  String path = url;
  String queries = "";

  String payload = ( method + "\n" + nonce + "\n" + date + "\n" + contentType
      + "\n" + path + "\n" + queries + "\n"
    );
  payload.toLowerCase();

  String auth = "On " + _accessKey + ":HmacSHA256:" + getHmac64(payload, _secretKey);
  
  return auth;
}

// TODO: Connection: keep-alive only connect to server on setup (for faster requests)
// https://github.com/espressif/arduino-esp32/issues/653#issuecomment-425645575
// https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=&cad=rja&uact=8&ved=2ahUKEwij3Omlq87qAhWodN8KHQLAC8oQFjAAegQIAhAB&url=https%3A%2F%2Fforum.arduino.cc%2Findex.php%3Ftopic%3D376619.0&usg=AOvVaw3kdXk3RHeCWh5Q4SDj4Duc
String OnShape::makeRequest(String reqType, String path) {
  if (_client.connected()) {
      String onNonce = buildNonce();
      String date = utcString();
      String signature = buildHeaders( reqType, path, onNonce, date, "application/json" );
      
      // Make a HTTP request:
      _client.println(reqType + " " + path + " HTTP/1.1");
      _client.println("Host: cad.onshape.com");
      _client.println("Connection: keep-alive");
      _client.println("Keep-Alive: timeout=5, max=100");
      
      _client.println("Content-Type: application/json");
      _client.println("Date: " + date);
      _client.println("On-Nonce: " + onNonce);
      _client.println("Authorization: " + signature);
      _client.println("Accept: application/vnd.onshape.v1+json");
  
      _client.println();
  
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

      return response;
  } else {
    _client.connect(server, 443);
    return "no connection.";
  }
}

/* POST DATA */
String OnShape::makeRequest(String reqType, String path, const char data[]) {
  if (_client.connected()) {
    String onNonce = buildNonce();
    String date = utcString();
    String signature = buildHeaders( reqType, path, onNonce, date, "application/json" );
    
    // Make a HTTP request:
    _client.println(reqType + " " + path + " HTTP/1.1");
    _client.println("Host: cad.onshape.com");
    _client.println("Connection: keep-alive");
    _client.println("Keep-Alive: timeout=5, max=100");
    _client.println("Accept: application/vnd.onshape.v1+json;charset=UTF-8;qs=0.1");
    
    _client.println("Content-Type: application/json");
    _client.print("Content-Length: ");
    _client.println(strlen(data));
    _client.println("Date: " + date);
    _client.println("On-Nonce: " + onNonce);
    _client.println("Authorization: " + signature);
    _client.println("Accept: application/vnd.onshape.v1+json");  
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
      return "no connection";
  }
}

/* WRAPPER FUNCTIONS */

String OnShape::getDocument(String documentID) {
	return makeRequest("GET", "/api/documents/"+documentID);
}