#include <heltec.h>
#include <TinyGPS++.h>

// Brazil LoRa frequency
#define BAND 915E6

TinyGPSPlus gps;

void setup() {
  pinMode(4, INPUT_PULLUP);
  pinMode(15, INPUT_PULLUP);
  Wire.setPins(4, 15);

  Heltec.begin(true, true, true);

  if (!LoRa.begin(BAND, true)) {
    Heltec.display->clear();
    Heltec.display->drawString(0, 0, "Erro LoRa!");
    Heltec.display->display();
    while (1);
  }

  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "Aguardando GPS...");
  Heltec.display->display();
}

void loop() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    gps.encode(c);

    if (gps.location.isUpdated()) {
      float lat = gps.location.lat();
      float lng = gps.location.lng();

      String payload = String(lat, 6) + "," + String(lng, 6);

      LoRa.beginPacket();
      LoRa.print(payload);
      LoRa.endPacket();

      Heltec.display->clear();
      Heltec.display->drawString(0, 0, "Enviado:");
      Heltec.display->drawString(0, 10, "Lat: " + String(lat, 6));
      Heltec.display->drawString(0, 20, "Lng: " + String(lng, 6));
      Heltec.display->display();

      delay(2000);
    }
  }
}
