#include <OnShape.h>

// "/api/documents" returns data that is too big to fit in memory, so we use makeRequestStream to display the JSON in Serial.
// From here, you could have a program listening to the Serial port, ready to receive/store the information, for example.

OnShape myclient("ZuQDyqNdTg0sZbuyOyLEY2B4", "gOaFuk0r5IDTCd03tYXlJP8c5KP6lTgZj2bA1l8gzVRtDKz4");

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Your WIFI credentials go here
  myclient.start("Sinha-2.4_EXT", "296concordroad");
}

void loop() {
  WiFiSSLClient response = myclient.makeRequestStream("GET", "/api/documents");
  do {
    Serial.write(response.read());
  } while(response.available());

  delay(10000);
}