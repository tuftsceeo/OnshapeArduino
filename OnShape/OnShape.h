#ifndef OnShape_h
#define OnShape_h

#include "Arduino.h"

#include <SPI.h>
#include <WiFiNINA.h>
#include <TimeLib.h>

#include <sha256.h>

class OnShape
{
	public:
		OnShape(const char accessKey[], const char secretKey[]);
		void start(const char ssid[], const char pass[]);

		/* Wrapper functions */
		String getDocument(const char documentID[]);
		String newDocument(const char docName[]);

		/* Raw request functions (use these if no wrapper function exists) */
		String makeRequest(const char reqType[], const char path[]);
		String makeRequest(const char reqType[], const char path[], const char data[]);

		/* Request stream functions (for large data that can't fit in memory) */
		WiFiSSLClient makeRequestStream(const char reqType[], const char path[]);
		WiFiSSLClient makeRequestStream(const char reqType[], const char path[], const char data[]);
	private:
		const char *_accessKey;
		const char *_secretKey;
		WiFiSSLClient _client;

		void connectWifi(const char ssid[], const char pass[]);
		void printWifiStatus();

		void buildNonce(char* out);
		void utcString(char* out);
		void getHmac64(const char payload[], const char key[], char* out);
		void buildHeaders (const char method[], const char url[], const char nonce[], const char date[], const char contentType[], char* out);
		
};

#endif