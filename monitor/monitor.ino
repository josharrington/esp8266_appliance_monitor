#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#define SHAKING 1
#define NOT_SHAKING 0
#define ESP_D0 16

// ===== User Configurables =====

#define POLLRATE 50
#define GRACEPERIOD 120000
#define ALERTTHRESHOLD 60000
const char wifi_ssid[] = "";
const char wifi_password[] = "";
const String pushingbox_device_id = "";

// ===== End User Configurables =====

bool shaking = false;
bool shaking_finished = false;
int last_shake_ms;
ESP8266WiFiMulti WiFiMulti;

void ShakeStarted() {
  last_shake_ms = millis();
  Serial.println("Shaking detected at " + String(last_shake_ms) + " ms");
  shaking = true;
}

void ShakeFinished(){
  Serial.println("=== Shaking finished at " + String(millis()) + " ===");
  shaking_finished = true;
  digitalWrite(LED_BUILTIN, LOW);
  SendAlert();
}

void SendAlert() {
  if (wifi_ssid == "" || wifi_password == "") {
    Serial.println("Wifi is not set up. Please configure the variables before continuing");
  }

  Serial.println("Connecting to Wifi and sending alert");
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(wifi_ssid, wifi_password);
  HTTPClient http;
  WiFiClient client;

  if (WiFiMulti.run() != WL_CONNECTED) {
    Serial.println("Failed to connect to wifi");
    return;
  }

  Serial.println("Connected!");

  http.begin(client, "http://api.pushingbox.com/pushingbox?devid=" + pushingbox_device_id);
  int httpCode = http.GET();
  Serial.println("HTTP Response from API: " + String(httpCode));
  http.end();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output

  Serial.begin(115200);
  Serial.println("");
  Serial.flush();
  Serial.println("Waiting for grace period to expire...");

  while (millis() < GRACEPERIOD) {
    // Cycle LED and wait
    digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) ? LOW : HIGH);
    delay(200);
  }

  digitalWrite(LED_BUILTIN, HIGH);  // Turn LED off
  Serial.println("Beginning Shake Detection");
}

void loop() {
  delay(POLLRATE);

  if (shaking_finished) {
    Serial.println("Shaking Finished, waiting");
    delay(1000000);
    return;
  }

  if (digitalRead(ESP_D0) == SHAKING) {
    ShakeStarted();
  }
  else if (millis() - last_shake_ms > ALERTTHRESHOLD && shaking) {
    ShakeFinished();
  }
}
