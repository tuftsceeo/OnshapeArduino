#include <OnShape.h>

// When a button is pressed, it will change the color of a component in my OnShape document

const int BUTTON_PIN = 10;

// Your Access Key and Secret Key go here
OnShape myclient("ZuQDyqNdTg0sZbuyOyLEY2B4", "gOaFuk0r5IDTCd03tYXlJP8c5KP6lTgZj2bA1l8gzVRtDKz4");

void setup() {
  pinMode(BUTTON_PIN, INPUT);

  Serial.begin(9600);
  while (!Serial);

  // Your WIFI credentials go here
  myclient.start("Sinha-2.4_EXT", "296concordroad");

  // Get document info from document id
  Serial.println(myclient.getDocument("4e7fba700f1180e3f3befacf"));
}

int prevVal = 0;

const char redinput[] = "{\"items\":[{\"href\":\"https://cad.onshape.com/api/metadata/d/4e7fba700f1180e3f3befacf/w/3648faa98085565850471de3/e/8c27f3aa9b04e06bf7647cc6/p/JHD?configuration=default\",\"properties\":[{\"propertyId\":\"57f3fb8efa3416c06701d60c\",\"value\":{\"isGenerated\":false,\"color\":{\"red\":255,\"green\":0,\"blue\":0},\"opacity\":255}}]}]}";
const char grayinput[] = "{\"items\":[{\"href\":\"https://cad.onshape.com/api/metadata/d/4e7fba700f1180e3f3befacf/w/3648faa98085565850471de3/e/8c27f3aa9b04e06bf7647cc6/p/JHD?configuration=default\",\"properties\":[{\"propertyId\":\"57f3fb8efa3416c06701d60c\",\"value\":{\"isGenerated\":false,\"color\":{\"red\":165,\"green\":165,\"blue\":165},\"opacity\":255}}]}]}";

void loop () {
  int val = digitalRead(BUTTON_PIN);
  if (val != prevVal) {
    Serial.println("Button is" + String(val ? "" : " not") + " pressed!");
    if (val) {
      Serial.println(myclient.makeRequest("POST", "/api/metadata/d/4e7fba700f1180e3f3befacf/w/3648faa98085565850471de3/e/8c27f3aa9b04e06bf7647cc6/p/JHD", redinput));
    } else {
      Serial.println(myclient.makeRequest("POST", "/api/metadata/d/4e7fba700f1180e3f3befacf/w/3648faa98085565850471de3/e/8c27f3aa9b04e06bf7647cc6/p/JHD", grayinput));
    }
    Serial.println();
  }
  prevVal = val;
}
