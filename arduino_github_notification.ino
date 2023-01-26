#include "config.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFiMulti.h>

// Add config in config.h
const char wifi_ssid[] = WIFI_SSID;
const char wifi_password[] = WIFI_PASSWORD;
const char wifi_ssid2[] = WIFI_SSID2;
const char wifi_password2[] = WIFI_PASSWORD2;

ESP8266WiFiMulti wifiMulti;
const uint32_t connectTimeoutMs = 5000;

const char endpoint[] = ENDPOINT;
const String token = TOKEN;

String saved_sha = "";

WiFiClientSecure client;
HTTPClient httpsClient;

StaticJsonDocument<16> filter_sha;
StaticJsonDocument<64> doc;

const uint8_t BUZZER = 4;
const uint8_t WIFI_LED = 5;
const uint8_t COMMIT_LED = 12;

// wifi status
int status = WL_IDLE_STATUS;

void setup()
{
    Serial.begin(9600);

    // turn off build in led
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    
    pinMode(BUZZER, OUTPUT);
    pinMode(WIFI_LED, OUTPUT);
    pinMode(COMMIT_LED, OUTPUT);

    WiFi.mode(WIFI_STA);

    wifiMulti.addAP(wifi_ssid, wifi_password);
    wifiMulti.addAP(wifi_ssid2, wifi_password2);    
    
    filter_sha["sha"] = true;
    
    client.setInsecure();
}

void loop()
{ 
if (wifiMulti.run(connectTimeoutMs) == WL_CONNECTED) {
    Serial.print("WiFi connected: ");
    Serial.print(WiFi.SSID());
    Serial.print(" ");
    Serial.println(WiFi.localIP());
    digitalWrite(WIFI_LED, HIGH);
 
    if(checkForCommits()) {
      playBuzzer();
      flashLed();
    }


    } else {
    Serial.println("WiFi not connected!");
  }
    delay(5000);
}

boolean checkForCommits() {
  httpsClient.begin(client, endpoint);
    httpsClient.addHeader("Authorization", "token " + token);
    httpsClient.GET();
    deserializeJson(doc, httpsClient.getStream(), DeserializationOption::Filter(filter_sha));
   
    //serializeJsonPretty(doc, Serial); // Print debug of response
    
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

          //playBuzzer();
          //flashLed();
          return true;
          Serial.println("Updated...");
      }
      return false;
      Serial.println("Nothing to do");
    }
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
