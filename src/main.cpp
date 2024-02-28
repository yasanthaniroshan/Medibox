#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include "time.h"

#define SCREEN_WIDTH 128
#define SCREEN_HIEGHT 64
#define OLED_RESET 4
#define SCREEN_ADDRESS 0x3C
#define LED 2

#define MenuInterruptPin 35
#define I2C_SDA 33
#define I2C_SCL 32
#define DHTPIN 15     // DHT22 signal pin is connected to GPIO 4
#define DHTTYPE DHT22 // DHT22 (AM2302)

typedef struct
{
    float temperature;
    float humidity;
} DHTData;

typedef struct
{
    bool pressed;
    int millisPressed;
} Button;

typedef struct
{
    bool pressed;
    byte numberKeyPresses;
} Menu;

Button menuButton = {false, 0};
Menu menu = {false, 0};

const char *ssid = "MSI 8690";
const char *password = "abcdefgh";

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

void intializeDisplay();
void displayText(String text);
void DHTInit();
void wifiInit();

struct tm timeinfo;
DHTData dhtData;
TwoWire wireInterfaceDisplay = TwoWire(0);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HIEGHT, &wireInterfaceDisplay, OLED_RESET);
DHT dht(DHTPIN, DHTTYPE);

void wifiInit()

{
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected.");

    // Init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    // WiFi.disconnect(true);
}

void intializeDisplay()
{
    wireInterfaceDisplay.begin(I2C_SDA, I2C_SCL, 100000);
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        for (;;)
            ;
    }
    display.display();
    delay(500);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Hello");
    display.display();
}

void displayText(String text)
{

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(text);
    display.display();
}

void displayData()
{
    display.clearDisplay();
    display.setCursor(0, 48);
    display.println("Temperature: " + String(dhtData.temperature) + "C");
    display.setCursor(0, 56);
    display.println("Humidity: " + String(dhtData.humidity) + "%");
    display.setCursor(0, 0);
    display.println(&timeinfo, "%d-%m-%Y");
    display.setCursor(0, 8);
    display.setTextSize(2);
    display.println(&timeinfo, "%H:%M:%S");
    display.setTextSize(1);

    display.display();
}

void DHTInit()
{
    // Initialize DHT sensor
    dht.begin();
}

void measureDHT()
{

    dhtData.temperature = dht.readTemperature();
    dhtData.humidity = dht.readHumidity();

    if (isnan(dhtData.temperature) || isnan(dhtData.humidity))
    {
        Serial.println("Failed to read data from DHT22 sensor");
        return;
    }
}

void getTime()
{
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return;
    }
}

void IRAM_ATTR menuISR() {
    menuButton.millisPressed = millis();
    menuButton.pressed = true;
}

void displayMenu()
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Menu");
    display.setCursor(0, 8);
    display.println("%d Presses: "+String(menuButton.millisPressed));
    display.display();
}

void setup()
{
    Serial.begin(112500);
    pinMode(LED, OUTPUT);
    pinMode(MenuInterruptPin, INPUT_PULLUP);
    attachInterrupt(MenuInterruptPin, menuISR, RISING);
    intializeDisplay();
    DHTInit();
    wifiInit();
}

void loop()
{
    measureDHT();
    displayData();
    getTime();
    if(menuButton.pressed && (millis() - menuButton.millisPressed) > 100)
    {
        displayMenu();
        delay(5000);
        menuButton.pressed = false;
    }
}

