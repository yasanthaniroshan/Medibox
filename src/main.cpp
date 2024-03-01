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
#define BUZZER 23

#define MenuInterruptPin 27
#define GoForwardInterruptPin 26
#define GoBackwardInterruptPin 25
#define CancelInterruptPin 35

#define I2C_SDA 33
#define I2C_SCL 32
#define DHTPIN 15     // DHT22 signal pin is connected to GPIO 4
#define DHTTYPE DHT22 // DHT22 (AM2302)

#define TEMPURATION_LOWER_LIMIT 26
#define TEMPURATION_UPPER_LIMIT 32

#define HUMIDITY_LOWER_LIMIT 60
#define HUMIDITY_UPPER_LIMIT 80

typedef struct
{
    float temperature;
    float humidity;
} DHTData;

typedef struct
{
    bool pressed;
    int millisPressed;
    int numberKeyPresses;
} Button;

typedef struct
{
    bool pressed;
    bool isPopedUp;
    bool isClosed;
    bool isSubMenuClosed;
    byte numberKeyPresses;

} Menu;

typedef struct
{
    void *selectedOption(void);
    byte frameStartY;
} SelectedFrame;
typedef struct
{
    byte alarm_id;
    char *alarm_name;
    byte hours;
    byte minutes;
} AlarmTime;

typedef enum
{
    Alarm_01 = 0,
    Alarm_02 = 1,
    Alarm_03 = 2,
    SetOffset = 3,
    DisableAllAlarms = 4
} MenuOptions;

MenuOptions selectedOption = Alarm_01;

SelectedFrame selectedFrame = {10};
Button menuButton = {false, 0, 0};
Button goForwardButton = {false, 0, 0};
Button goBackwardButton = {false, 0, 0};
Button cancelButton = {false, 0, 0};
Menu menu = {false, false, false, false, 0};

AlarmTime alarms[3] = {{0, "Alarm-01", 0, 0}, {0, "Alarm-02", 0, 0}, {0, "Alarm-03", 0, 0}};
AlarmTime offset = {0, "Offset", 5, 30};

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
void IRAM_ATTR timer_isr()
{
    digitalWrite(BUZZER, HIGH);
}

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
    // WiFi.setSleep(WIFI_PS_MAX_MODEM);
}

void setTime()
{
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
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
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(22, 24);
    display.println("MediBoX");
    display.display();
    delay(1000);
    display.setTextSize(1);
}

void setAlarm(AlarmTime *alarm)
{
    hw_timer_t *timer = NULL;
    uint8_t timer_id = 0;
    uint16_t prescaler = 40000;
    int threshold = 2000;
    uint64_t alarmTime;
    getTime(&timeinfo);
    int timeNowInSeconds = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;
    int alarmTimeInSeconds = (alarm->hours * 3600 + alarm->minutes * 60);
    if (alarmTimeInSeconds < timeNowInSeconds)
    {
        alarmTime = 86400 - timeNowInSeconds + alarmTimeInSeconds;
    }
    else
    {
        alarmTime = alarmTimeInSeconds - timeNowInSeconds;
    }
    alarmTime = alarmTime * threshold;
    timer = timerBegin(timer_id, prescaler, true);
    timerAttachInterrupt(timer, &timer_isr, true);
    timerAlarmWrite(timer, alarmTime, true);
    timerAlarmEnable(timer);
}

void changeTimeZone(AlarmTime *alarm)
{
    int offset;
    if (alarm->hours > -12 && alarm->hours < 0)
    {
        offset = alarm->hours * 3600 - alarm->minutes * 60;
    }
    else if (alarm->hours > 0 && alarm->hours < 14)
    {
        offset = alarm->hours * 3600 + alarm->minutes * 60;
    }
    else if (alarm->hours == -12)
    {
        offset = alarm->hours * 3600;
    }
    else if (alarm->hours == 14)
    {
        offset = alarm->hours * 3600;
    }
    configTime(offset, 0, ntpServer);
}

void displayText(String text)
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(text);
    display.display();
}

void displayData(tm timeinfo, DHTData dhtData)
{
    display.clearDisplay();
    display.setCursor(0, 48);
    display.drawRect(12, 14, 104, 28, SSD1306_WHITE);
    display.println("Temperature: " + String(dhtData.temperature) + "C");
    display.setCursor(0, 56);
    display.println("Humidity: " + String(dhtData.humidity) + "%");
    display.setCursor(34, 0);
    display.println(&timeinfo, "%d-%m-%Y");
    display.setCursor(16, 20);
    display.setTextSize(2);
    display.println(&timeinfo, "%H:%M:%S");
    display.setTextSize(1);
    display.display();
}
void displayAlarm(byte data, String text)
{
    display.clearDisplay();
    display.setCursor(45, 2);
    display.setTextSize(1);
    display.println("Set " + text);
    display.setTextSize(3);
    display.setCursor(0, 26);
    display.println("+");
    display.setCursor(55, 26);
    display.println(data);
    display.setCursor(110, 26);
    display.println("-");
    display.display();
    display.setTextSize(1);
}
void handleAlarm(AlarmTime *alarm, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton)
{
    displayAlarm(alarm->hours, "Hours");
    while (!menu.isSubMenuClosed)
    {
        adjustAlarmHours(alarm, menuButton, goForwardButton, goBackwardButton, cancelButton);
    }
    menu.isSubMenuClosed = false;
    displayAlarm(alarm->minutes, "Minutes");
    while (!menu.isSubMenuClosed)
    {
        adjustAlarmMinutes(alarm, menuButton, goForwardButton, goBackwardButton, cancelButton);
    }
    menu.isSubMenuClosed = true;
    setAlarm(alarm);
}
void handleTimeZone(AlarmTime *alarm, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton)
{
    displayAlarm(alarm->hours, "Hours");
    while (!menu.isSubMenuClosed)
    {
        adjustTimeZoneHours(alarm, menuButton, goForwardButton, goBackwardButton, cancelButton);
    }
    menu.isSubMenuClosed = false;
    displayAlarm(alarm->minutes, "Minutes");
    while (!menu.isSubMenuClosed)
    {
        adjustTimeZoneMinutes(alarm, menuButton, goForwardButton, goBackwardButton, cancelButton);
    }
    menu.isSubMenuClosed = true;
    changeTimeZone(alarm);
}
void adjustAlarmHours(AlarmTime *alarm, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton)
{
    pooling(goForwardButton, goBackwardButton, cancelButton);
    if (goForwardButton->pressed)
    {
        if (alarm->hours < 23)
        {
            alarm->hours++;
        }
        else
        {
            alarm->hours = 0;
        }
        goForwardButton->pressed = false;
        goForwardButton->numberKeyPresses = 0;
        displayAlarm(alarm->hours, "Hours");
    }
    else if (goBackwardButton->pressed)
    {
        if (alarm->hours > 0)
        {
            alarm->hours--;
        }
        else
        {
            alarm->hours = 23;
        }
        goBackwardButton->pressed = false;
        goBackwardButton->numberKeyPresses = 0;
        displayAlarm(alarm->hours, "Hours");
    }
    else if (cancelButton->pressed)
    {
        menu.isSubMenuClosed = true;
        cancelButton->pressed = false;
        cancelButton->numberKeyPresses = 0;
    }
    else if (menuButton->pressed && debounce(menuButton))
    {
        menu.isSubMenuClosed = true;
        menuButton->pressed = false;
        menuButton->numberKeyPresses = 0;
    }
}
void adjustAlarmMinutes(AlarmTime *alarm, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton)
{
    pooling(goForwardButton, goBackwardButton, cancelButton);
    if (goForwardButton->pressed)
    {
        if (alarm->minutes < 59)
        {
            alarm->minutes++;
        }
        else
        {
            alarm->minutes = 0;
        }
        goForwardButton->pressed = false;
        goForwardButton->numberKeyPresses = 0;
        displayAlarm(alarm->minutes, "Minutes");
    }
    else if (goBackwardButton->pressed)
    {
        if (alarm->minutes > 0)
        {
            alarm->minutes--;
        }
        else
        {
            alarm->minutes = 59;
        }
        goBackwardButton->pressed = false;
        goBackwardButton->numberKeyPresses = 0;
        displayAlarm(alarm->minutes, "Minutes");
    }
    else if (cancelButton->pressed)
    {
        menu.isSubMenuClosed = true;
        cancelButton->pressed = false;
        cancelButton->numberKeyPresses = 0;
    }
    else if (menuButton->pressed && debounce(menuButton))
    {
        menu.isSubMenuClosed = true;
        menuButton->pressed = false;
        menuButton->numberKeyPresses = 0;
    }
}
void adjustTimeZoneHours(AlarmTime *alarm, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton)
{
    pooling(goForwardButton, goBackwardButton, cancelButton);
    if (goForwardButton->pressed)
    {
        if (alarm->hours > -13 && alarm->hours < 15)
        {
            alarm->hours++;
        }
        else if (alarm->hours >= 15)
        {
            alarm->hours = -13;
        }
        else if (alarm->hours <= -13)
        {
            alarm->hours = 15;
        }
        goForwardButton->pressed = false;
        goForwardButton->numberKeyPresses = 0;
        displayAlarm(alarm->hours, "Hours");
    }
    else if (goBackwardButton->pressed)
    {
        if (alarm->hours > -13 && alarm->hours < 15)
        {
            alarm->hours--;
        }
        else if (alarm->hours <= -13)
        {
            alarm->hours = 15;
        }
        else if (alarm->hours >= 15)
        {
            alarm->hours = -13;
        }
        goBackwardButton->pressed = false;
        goBackwardButton->numberKeyPresses = 0;
        displayAlarm(alarm->hours, "Hours");
    }
    else if (cancelButton->pressed)
    {
        menu.isSubMenuClosed = true;
        cancelButton->pressed = false;
        cancelButton->numberKeyPresses = 0;
    }
    else if (menuButton->pressed && debounce(menuButton))
    {
        menu.isSubMenuClosed = true;
        menuButton->pressed = false;
        menuButton->numberKeyPresses = 0;
    }
}
void adjustTimeZoneMinutes(AlarmTime *alarm, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton)
{
    pooling(goForwardButton, goBackwardButton, cancelButton);
    if (goForwardButton->pressed)
    {
        if (alarm->minutes < 59)
        {
            alarm->minutes++;
        }
        else
        {
            alarm->minutes = 0;
        }
        goForwardButton->pressed = false;
        goForwardButton->numberKeyPresses = 0;
        displayAlarm(alarm->minutes, "Minutes");
    }
    else if (goBackwardButton->pressed)
    {
        if (alarm->minutes > 0)
        {
            alarm->minutes--;
        }
        else
        {
            alarm->minutes = 59;
        }
        goBackwardButton->pressed = false;
        goBackwardButton->numberKeyPresses = 0;
        displayAlarm(alarm->minutes, "Minutes");
    }
    else if (cancelButton->pressed)
    {
        menu.isSubMenuClosed = true;
        cancelButton->pressed = false;
        cancelButton->numberKeyPresses = 0;
    }
    else if (menuButton->pressed && debounce(menuButton))
    {
        menu.isSubMenuClosed = true;
        menuButton->pressed = false;
        menuButton->numberKeyPresses = 0;
    }
}
void handleMenu(MenuOptions selectedOption, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton)
{
    if (selectedOption == Alarm_01 || selectedOption == Alarm_02 || selectedOption == Alarm_03)
    {
        menu.isSubMenuClosed = false;
        handleAlarm(&alarms[selectedOption], menuButton, goForwardButton, goBackwardButton, cancelButton);
        displayMenu(&selectedFrame, &menu);
        return;
    }
    else if (selectedOption == SetOffset)
    {
        menu.isSubMenuClosed = false;
        handleTimeZone(&offset, menuButton, goForwardButton, goBackwardButton, cancelButton);
        displayMenu(&selectedFrame, &menu);
    }
    else if (selectedOption == DisableAllAlarms)
    {
    }
}
void DHTInit()
{
    // Initialize DHT sensor
    dht.begin();
}

void measureDHT(DHTData *dhtData)
{

    dhtData->temperature = dht.readTemperature();
    dhtData->humidity = dht.readHumidity();

    if (isnan(dhtData->temperature) || isnan(dhtData->humidity))
    {
        Serial.println("Failed to read data from DHT22 sensor");
        return;
    }
}

void getTime(tm *timeinfo)
{
    if (!getLocalTime(timeinfo))
    {
        Serial.println("Failed to obtain time");
        return;
    }
}

void IRAM_ATTR menuISR()
{
    menuButton.millisPressed = millis();
    menuButton.pressed = true;
}

bool debounce(Button *button)
{
    if (button->pressed && (millis() - button->millisPressed) > 100)
    {
        return true;
    }
    return false;
}
void displayMenu(SelectedFrame *selectedFrame, Menu *menu)
{
    menu->isPopedUp = true;
    display.clearDisplay();
    display.drawRect(0, selectedFrame->frameStartY, 128, 10, SSD1306_WHITE);
    display.setCursor(52, 2);
    display.println("Menu");
    display.setCursor(0, 12);
    display.println("1. Set Alarm 01");
    display.setCursor(0, 22);
    display.println("2. Set Alarm 02");
    display.setCursor(0, 32);
    display.println("3. Set Alarm 03");
    display.setCursor(0, 42);
    display.println("4. Set Offset");
    display.setCursor(0, 52);
    display.println("5. Disable all alarms");
    display.display();
}

void pooling(Button *goForwardButton, Button *goBackwardButton, Button *cancelButton)
{

    if (!digitalRead(GoForwardInterruptPin))
    {
        goForwardButton->pressed = true;
        goForwardButton->numberKeyPresses++;
    }
    else if (!digitalRead(GoBackwardInterruptPin))
    {
        goBackwardButton->pressed = true;
        goBackwardButton->numberKeyPresses++;
    }
    else if (!digitalRead(CancelInterruptPin))
    {
        cancelButton->pressed = true;
        cancelButton->numberKeyPresses++;
    }
    delay(100);
}

bool handleMainMenu(SelectedFrame *selectedFrame, Menu *menu, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton)
{
    pooling(goForwardButton, goBackwardButton, cancelButton);
    if (goForwardButton->pressed)
    {
        if (selectedFrame->frameStartY < 50)
        {
            selectedFrame->frameStartY += 10;
            selectedOption = (MenuOptions)(selectedOption + 1);
        }
        else
        {
            selectedFrame->frameStartY = 10;
            selectedOption = Alarm_01;
        }
        goForwardButton->pressed = false;
        goForwardButton->numberKeyPresses = 0;
        displayMenu(selectedFrame, menu);
    }
    else if (goBackwardButton->pressed)
    {
        if (selectedFrame->frameStartY > 10)
        {
            selectedFrame->frameStartY -= 10;
            selectedOption = (MenuOptions)(selectedOption - 1);
        }
        else
        {
            selectedFrame->frameStartY = 50;
            selectedOption = DisableAllAlarms;
        }
        goBackwardButton->pressed = false;
        goBackwardButton->numberKeyPresses = 0;
        displayMenu(selectedFrame, menu);
    }
    else if (cancelButton->pressed)
    {
        menu->isClosed = true;
        cancelButton->pressed = false;
        cancelButton->numberKeyPresses = 0;
        return true;
    }
    else if (menuButton->pressed && debounce(menuButton))
    {
        menuButton->numberKeyPresses = 0;
        menuButton->pressed = false;
        handleMenu(selectedOption, menuButton, goForwardButton, goBackwardButton, cancelButton);
        return true;
    }
    return false;
}

void ringAlarm()
{
    digitalWrite(BUZZER, HIGH);
}

void setup()
{
    Serial.begin(112500);
    pinMode(LED, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    pinMode(MenuInterruptPin, INPUT_PULLUP);
    pinMode(CancelInterruptPin, INPUT);
    pinMode(GoForwardInterruptPin, INPUT);
    pinMode(GoBackwardInterruptPin, INPUT);
    attachInterrupt(MenuInterruptPin, menuISR, RISING);
    wifiInit();
    setTime();
    DHTInit();
    delay(2000);
    intializeDisplay();
    // esp_sleep_enable_timer_wakeup(1 * 1000000); // light sleep for 2 seconds
    // gpio_wakeup_enable(GPIO_NUM_35, GPIO_INTR_LOW_LEVEL);
    // esp_sleep_enable_gpio_wakeup();
    // esp_light_sleep_start();
}

void loop()
{
    measureDHT(&dhtData);
    displayData(timeinfo, dhtData);
    getTime(&timeinfo);
    if (menuButton.pressed && debounce(&menuButton))
    {
        menuButton.pressed = false;
        displayMenu(&selectedFrame, &menu);
        while (!menu.isClosed)
        {
            bool responce = handleMainMenu(&selectedFrame, &menu, &menuButton, &goForwardButton, &goBackwardButton, &cancelButton);
        }
        menu.isClosed = false;
    }
    displayData(timeinfo, dhtData);
}
