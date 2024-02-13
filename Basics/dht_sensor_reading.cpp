#include <Arduino.h>
#include <DHTesp.h>

#define DHTPIN 15


DHTesp dht;


void setup() {
  Serial.begin(9600);
  dht.setup(DHTPIN,DHTesp::DHT22); // data pin 2, DHT22
}

void loop() {
  float h = dht.getHumidity();
  float t = dht.getTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C");
  delay(2000);
}
