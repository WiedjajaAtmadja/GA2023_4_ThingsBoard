#include <Arduino.h>
#include <WiFi.h>
#include <DHTesp.h>
#include <BH1750.h>
#include <ThingsBoard.h>
#include "devices.h"
#define WIFI_SSID "iot2.4g"
#define WIFI_PASSWORD "iot2.4g123"

#define THINGSBOARD_SERVER "demo.thingsboard.io"
#define THINGSBOARD_ACCESS_TOKEN "jxkcbrmifget2elqyzpb"
DHTesp dht;
BH1750 lightMeter;
WiFiClient wifiClient;
ThingsBoard tb(wifiClient);

void WifiConnect()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }  
  Serial.print("System connected with IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("RSSI: %d\n", WiFi.RSSI());
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  dht.setup(DHT_PIN, DHTesp::DHT11);
  pinMode(PUSH_BUTTON, INPUT_PULLUP);
  Wire.begin();
  lightMeter.begin(); 
  Serial.println("System ready!");
  WifiConnect();
  if (tb.connect(THINGSBOARD_SERVER, THINGSBOARD_ACCESS_TOKEN))
    Serial.println("Connected to ThingsBoard");
  else
    Serial.println("Error connected to ThingsBoard");
}

unsigned long nTimerHelpButton = 0;
unsigned long nTimerSendData = 0;
void loop() {
  unsigned long nNow;

  nNow = millis();
  if ((nNow - nTimerHelpButton)>5000) // tiap 5 detik lakukan:
  {
    nTimerHelpButton = nNow;
    if (digitalRead(PUSH_BUTTON)==LOW)
    {
      Serial.println("Button pressed!");
      tb.sendTelemetryBool("helpButton", true);
    }
    else
    {
      tb.sendTelemetryBool("helpButton", false);
    }
  }

  nNow = millis();
  if (nNow - nTimerSendData>10000) // tiap 10 detik lakukan:
  {
    digitalWrite(LED_BUILTIN, HIGH);
    nTimerSendData = nNow;
    Serial.println("Send Data to FIrebase");

    float fHumidity = dht.getHumidity();
    float fTemperature = dht.getTemperature();
    float lux = lightMeter.readLightLevel();
    Serial.printf("Humidity: %.2f, Temperature: %.2f, Light: %.2f \n",
        fHumidity, fTemperature, lux);
    tb.sendTelemetryFloat("humidity", fHumidity);
    tb.sendTelemetryFloat("temperature", fTemperature);
    tb.sendTelemetryFloat("light", lux);
    digitalWrite(LED_BUILTIN, LOW);
  }
  tb.loop();
}