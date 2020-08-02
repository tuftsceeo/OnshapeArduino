#ifndef OnShape_h
#define OnShape_h

#include "Arduino.h"

#include <SPI.h>
#include <WiFiNINA.h>
#include <TimeLib.h>

#include <sha256.h>
#include <base64.hpp>

class OnShape
{
	public:
		OnShape(String accessKey, String secretKey, char ssid[], char pass[]);
	private:
		String _accessKey;
		String _secretKey;
		WiFiSSLClient _client;

		void connectWifi(char ssid[], char pass[]);
		void printWifiStatus();

		String buildNonce();
		String utcString();
		String getHmac64(String payload, String key);
		String buildHeaders(String method, String urlpath, String nonce, String date, String contentType);
		
		void makeRequest(String reqType, String path);
		void makeRequest(String reqType, String path, const char data[]);
};

#endif