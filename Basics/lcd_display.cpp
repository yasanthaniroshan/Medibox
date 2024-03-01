#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HIEGHT 64
#define OLED_RESET 4
#define SCREEN_ADDRESS 0x3C

#define I2C_SDA 33
#define I2C_SCL 32

TwoWire wireInterfaceDisplay = TwoWire(0);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HIEGHT, &wireInterfaceDisplay, OLED_RESET);

void setup()
{
    Serial.begin(112500);
    wireInterfaceDisplay.begin(I2C_SDA, I2C_SCL, 100000);
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println("SSD1306 allocation failed");
        for (;;)
            ; // Don't proceed, loop forever
    }
    Serial.println("Connected !");
    display.display();
    delay(2000);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,10);
    display.println("Hello");
    display.display();



}

void loop()
{
}