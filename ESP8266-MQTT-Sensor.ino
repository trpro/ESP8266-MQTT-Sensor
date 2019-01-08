#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include "config.h"

#ifdef DHT_SENS
#include <DHT.h>
#endif // DHT_SENS

#ifdef DS18B20_SENS
#include <OneWire.h>
#include <DallasTemperature.h>
#endif // DS18B20_SENS

#ifdef DHT_SENS
DHT dht(DHTPin, DHTTYPE);
#endif // DHT_SENS

#ifdef DS18B20_SENS
OneWire oneWire(OneWirePin);
DallasTemperature DS18B20(&oneWire);
#endif // DS18B20_SENS

// wifi client for mqtt-connection
#ifndef MQTT_SECURE
WiFiClient client;
#endif // MQTT_SECURE

#ifdef MQTT_SECURE
WiFiClientSecure client;
#endif // MQTT_SECURE

Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_SERVERPORT, MQTT_USERNAME, MQTT_KEY);

Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, MQTT_TOPIC_TEMP(ROOM));
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, MQTT_TOPIC_HUMI(ROOM));

#ifdef BRIGHTNESS_SENS
Adafruit_MQTT_Publish brightness = Adafruit_MQTT_Publish(&mqtt, MQTT_TOPIC_BRIG(ROOM));
#endif

//void MQTT_connect();

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("MQTT Connecting... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         while (1);
       }
  }
  Serial.print(" connected!");
}
void setup() {
  Serial.begin(115200);

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to Wifi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

#ifdef DHT_SENS
  dht.begin();
#endif

  MQTT_connect();

  float t = -127;
#ifdef DHT_SENS
  t = dht.readTemperature(); 
  float h = dht.readHumidity();
#endif // DHT_SENS

#ifdef DS18B20_SENS
  for(int i = 0; i < 10; i++) {
    DS18B20.requestTemperatures(); 
    t = DS18B20.getTempCByIndex(0);
    if(t != 85.0 && t != -127)
      break;
  }
#endif // DS18B20_SENS

#ifdef BRIGHTNESS_SENS
  unsigned int b = analogRead(BrightnessPin);
#endif // BRIGHTNESS_SENS

  // publish to mqtt
  Serial.print(F("\nTemp: "));
  Serial.print(t);
  Serial.print("...");
  if (! temperature.publish(t)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

#ifdef DHT_SENS
  Serial.print(F("\nHumi: "));
  Serial.print(h);
  Serial.print("...");
  if (! humidity.publish(h)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

#endif // DHT_SENS


#ifdef BRIGHTNESS_SENS
  Serial.print(F("Brig:" ));
  Serial.print(b);
  Serial.print("...");
  if(!brightness.publish(b)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
#endif // BRIGHTNESS_SENS

  if(! mqtt.ping()) {
    mqtt.disconnect();
  }

  Serial.println("Going into deep sleep for 120 seconds");
  ESP.deepSleep(120e6); //120s
}

void loop() {
}

