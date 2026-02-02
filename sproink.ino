 #include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"


#define DHTPIN D3
#define DHTTYPE DHT11
#define Water D5
#define Fertilizer D6
#define Pesticide D6


#define WIFI_SSID "So Good"
#define WIFI_PASSWORD "helloworld"

#define API_KEY "AIzaSyC8791vESIJijViJt_gdfTbvmRAdb7qQ4k"
#define DATABASE_URL "sproink-368b5-default-rtdb.firebaseio.com/"


FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

DHT dht(DHTPIN, DHTTYPE);

bool signupOK = false;
unsigned long sendDataPrevMillis = 0;


void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(Water, OUTPUT);
  pinMode(Fertilizer, OUTPUT);
  pinMode(Pesticide, OUTPUT);
  digitalWrite(Water, HIGH);
  digitalWrite(Fertilizer, HIGH);
  digitalWrite(Pesticide, HIGH);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase connection successful");
    signupOK = true;
  
  } else {
    Serial.printf("Firebase sign-up error: %s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  Firebase.RTDB.setBool(&fbdo, "isConnected", true);

}

void loop() {
  if (Firebase.ready() && signupOK &&
      (millis() - sendDataPrevMillis > 200 || sendDataPrevMillis == 0)) {

    sendDataPrevMillis = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    Firebase.RTDB.setFloat(&fbdo, "SENSORS/1/humidity", h);
    Serial.println("Humidity" + h)
    Firebase.RTDB.setFloat(&fbdo, "SENSORS/1/temperature", t);
    Serial.println("Temperature" + t)
  

    if (Firebase.RTDB.getBool(&fbdo, "controls/waterRunning")) {
      if (fbdo.dataType() == "boolean"){
      bool waterStateStr = fbdo.boolData();
      Serial.println("Seccess: " + fbdo.dataPath() + ": " + waterStateStr + "(" + fbdo.dataType() + ")");
      bool water = (waterStateStr == false) ? HIGH : LOW;
      digitalWrite(Water, water);
      }
      
    } else {
      Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }
  
    if (Firebase.RTDB.getBool(&fbdo, "controls/pesticiedsRunning")) {
      if (fbdo.dataType() == "boolean"){
      bool pesticideStateStr = fbdo.boolData();
      Serial.println("Seccess: " + fbdo.dataPath() + ": " + pesticideStateStr + "(" + fbdo.dataType() + ")");
      bool pesticide = (pesticideStateStr == false) ? HIGH : LOW;
      digitalWrite(Pesticide, pesticide);
      }
      
    } else {
      Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }
  
    if (Firebase.RTDB.getBool(&fbdo, "controls/fertilizerRunning")) {
      if (fbdo.dataType() == "boolean"){
      bool fertilizerStateStr = fbdo.boolData();
      Serial.println("Seccess: " + fbdo.dataPath() + ": " + fertilizerStateStr + "(" + fbdo.dataType() + ")");
      bool fertilizer = (fertilizerStateStr == false) ? HIGH : LOW;
      digitalWrite(Fertilizer, fertilizer);
      }
      
    } else {
      Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }
  

  }
}
