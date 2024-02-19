#include <Arduino.h>
#include <DHTesp.h>


#define DHTPIN 15


DHTesp dht;


void readDHTSensor(DHTesp dht)
{
    
    float humidity = dht.getHumidity();
    float tempurature = dht.getTemperature();
    if (isnan(humidity)||isnan(tempurature))
    {
        Serial.println("Failed to get data from DHT sensor");
    }
    else
    {
        Serial.printf("Tempurature : %f\n",tempurature);
        Serial.printf("Humidity : %f\n",humidity);
    }
}


void setup() {
    dht.setup(DHTPIN,DHTesp::DHT22); // data pin 2, DHT22

  }

void loop() {
    readDHTSensor(dht);

  }
