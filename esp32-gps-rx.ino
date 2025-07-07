// Enable some features of Firebase
#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <heltec.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define API_KEY ""
#define DATABASE_URL ""
#define USER_EMAIL ""
#define USER_PASS ""

// Brazil LoRa frequency
#define BAND 915E6

UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASS);
FirebaseApp app;
WiFiClientSecure ssl_client;
AsyncClientClass aClient(ssl_client);
RealtimeDatabase Database;

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 5000;
String lastLat = "", lastLng = "";

void processResults(AsyncResult &aResult) {
  if (!aResult.isResult()) return;

  if (aResult.isEvent())
    Firebase.printf("Event: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());

  if (aResult.isError())
    Firebase.printf("Error: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());

  if (aResult.available())
    Firebase.printf("Data: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
}

void setup() {
  pinMode(4, INPUT_PULLUP);
  pinMode(15, INPUT_PULLUP);
  Wire.setPins(4, 15);

  Heltec.begin(true, true, true);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "Conectando Wi-Fi...");
  Heltec.display->display();

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
  }

  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "Wi-Fi Conectado");
  Heltec.display->display();

  ssl_client.setInsecure();
  ssl_client.setConnectionTimeout(1000);
  ssl_client.setHandshakeTimeout(5);

  initializeApp(aClient, app, getAuth(user_auth), processResults, "Auth");
  app.getApp<RealtimeDatabase>(Database);
  Database.url(DATABASE_URL);

  if (!LoRa.begin(BAND, true)) {
    Heltec.display->clear();
    Heltec.display->drawString(0, 0, "Erro LoRa!");
    Heltec.display->display();
    while (1);
  }

  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "Receptor inicializado!");
  Heltec.display->display();
}

void loop() {
  app.loop();

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String received = "";

    while (LoRa.available()) {
      received += (char)LoRa.read();
    }

    int commaIndex = received.indexOf(',');

    if (commaIndex > 0) {
      lastLat = received.substring(0, commaIndex);
      lastLng = received.substring(commaIndex + 1);

      Heltec.display->clear();
      Heltec.display->drawString(0, 0, "Recebido:");
      Heltec.display->drawString(0, 10, "Lat: " + lastLat);
      Heltec.display->drawString(0, 20, "Lng: " + lastLng);
      Heltec.display->display();
    }
  }

  static String lastSentLat = "";
  static String lastSentLng = "";

  if (app.ready()) {
    unsigned long currentTime = millis();
    if ((lastLat != "") && (currentTime - lastSendTime >= sendInterval)) {
      if (lastLat != lastSentLat || lastLng != lastSentLng) {
        lastSendTime = currentTime;
        lastSentLat = lastLat;
        lastSentLng = lastLng;

        Database.set<String>(aClient, "/gps/latitude", lastLat, processResults, "RTDB_Send_Lat");
        Database.set<String>(aClient, "/gps/longitude", lastLng, processResults, "RTDB_Send_Lng");

        Heltec.display->drawString(0, 30, "Enviado ao Firebase!");
        Heltec.display->display();
      }
    }
  }
}
