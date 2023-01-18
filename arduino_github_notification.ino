#include "config.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char wifi_ssid[] = WIFI_SSID;
const char wifi_password[] = WIFI_PASSWORD;
const char endpoint[] = ENDPOINT;
const String token = TOKEN;

String saved_sha = "";

WiFiClientSecure client;
HTTPClient httpsClient;

StaticJsonDocument<16> filter_sha;
StaticJsonDocument<64> doc;

const uint8_t BUZZER = 4;
const uint8_t WIFI_LED = 5;
const uint8_t COMMIT_LED = 2;

// wifi status
int status = WL_IDLE_STATUS;

void setup()
{
    pinMode(BUZZER, OUTPUT);
    pinMode(WIFI_LED, OUTPUT);
    pinMode(COMMIT_LED, OUTPUT);

    
    Serial.begin(9600);

    WiFi.mode(WIFI_STA);

    WiFi.begin(wifi_ssid, wifi_password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        
    }
    Serial.println("Connected");
    digitalWrite(WIFI_LED, HIGH);
    
    filter_sha["sha"] = true;

    client.setInsecure();
}

void loop()
{
    httpsClient.begin(client, endpoint);
    httpsClient.addHeader("Authorization", "token " + token);
    httpsClient.GET();
    deserializeJson(doc, httpsClient.getStream(), DeserializationOption::Filter(filter_sha));
    httpsClient.end();

    String sha = doc["sha"];
    
    if(saved_sha.equals("") && sha.length() > 5){
      Serial.println("Storing sha: "+sha);
      saved_sha = sha;
    } else {

      Serial.println("Saved_sha is: "+saved_sha);
      
      if(sha.length() > 5 && sha != saved_sha){
          Serial.println("New sha detected: "+sha);
          saved_sha = sha;

          playBuzzer();
          flashLed();
          

          Serial.println("Updated...");
      }

      Serial.println("Nothing to do");
    }

    delay(5000);
  
}

void playBuzzer() {
  tone(BUZZER, 1000);
  delay(750);
  noTone(BUZZER);
}

void flashLed() {
  for (int i=0;i<10;i++) {
    digitalWrite(COMMIT_LED, HIGH);
    delay(150);
    digitalWrite(COMMIT_LED, LOW);
    delay(150);
    }
}
