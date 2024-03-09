#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include "time.h"
#include <Preferences.h>
#include "bitmaps.h"

#define SCREEN_WIDTH 128
#define SCREEN_HIEGHT 64
#define OLED_RESET 4
#define SCREEN_ADDRESS 0x3C
#define LED 2
#define BUZZER 5

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
    bool isSet;
    bool isRinging;
    int64_t alarmTimeInSeconds;
    esp_timer_handle_t oneshot_timer;
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
AlarmTime alarms[3] = {{0, "Alarm-01", 0, 0, false, false, 0, NULL}, {1, "Alarm-02", 0, 0, false, false, 0, NULL}, {2, "Alarm-03", 0, 0, false, false, 0, NULL}};
AlarmTime offset = {0, "Offset", 5, 30, false, false, 0, NULL};

const char *ssid = "MSI 8690";
const char *password = "abcdefgh";

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;
bool ringingAlarm = false;
int timertime[3] = {0, 0, 0};

bool handleWarning(DHTData dhtData);
void intializeDisplay(TwoWire *wireInterfaceDisplay, Adafruit_SSD1306 *display);
void displayText(String text);
void messageDisplay(String title, String Message, unsigned char bitmap[]);
void DHTInit(DHT *dht);
void wifiInit(Adafruit_SSD1306 *display);
void ringAlarm();
void measureDHT(DHTData *dhtData);
void setTime(int gmtOffset_sec, int daylightOffset_sec, const char *ntpServer);
void intializeDisplay();
void loadAlarms(AlarmTime *alarms, Preferences *preferences);
void saveAlarm(AlarmTime *alarm, Preferences *preferences);
void setAlarm(AlarmTime *alarm);
void changeTimeZone(AlarmTime *alarm);
void displayText(String text);
void mainFaceDisplay(tm timeinfo, DHTData dhtData);
void displayAlarm(byte data, String text);
void handleAlarm(AlarmTime *alarm, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton);
void handleTimeZone(AlarmTime *alarm, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton);
void adjustAlarmHours(AlarmTime *alarm, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton);
void adjustAlarmMinutes(AlarmTime *alarm, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton);
void adjustTimeZoneHours(AlarmTime *alarm, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton);
void adjustTimeZoneMinutes(AlarmTime *alarm, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton);
void handleMenu(MenuOptions selectedOption, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton);
void measureDHT(DHTData *dhtData);
void getTime(tm *timeinfo);
bool debounce(Button *button);
void displayMenu(SelectedFrame *selectedFrame, Menu *menu);
void pooling(Button *goForwardButton, Button *goBackwardButton, Button *cancelButton);
bool handleMainMenu(SelectedFrame *selectedFrame, Menu *menu, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton);

struct tm timeinfo;
DHTData dhtData;
Preferences preferences;
TwoWire wireInterfaceDisplay = TwoWire(0);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HIEGHT, &wireInterfaceDisplay, OLED_RESET);
DHT dht(DHTPIN, DHTTYPE);

void isAlarm(AlarmTime alarms[])
{
    getTime(&timeinfo);
    for (int i = 0; i < 3; i++)
    {

        if (alarms[i].isSet && timeinfo.tm_hour == alarms[i].hours && timeinfo.tm_min == alarms[i].minutes)
        {
            alarms[i].isRinging = true;
            ringingAlarm = true;
            alarms[i].isSet = false;
        }
        else if (alarms[i].isSet && alarms[i].alarmTimeInSeconds > 0)
        {
            alarms[i].alarmTimeInSeconds--;
        }
    }
}

void wifiInit(Adafruit_SSD1306 *display)
{
    messageDisplay("WiFi", "Connecting..", wifi);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);
    }
    messageDisplay("WiFi", "Connected", wifi);
}

void setTime(int gmtOffset_sec, int daylightOffset_sec, const char *ntpServer)
{
    messageDisplay("Time", "Configuring..", timeConfig);
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    delay(350);
    messageDisplay("Time", "Configured", timeConfig);
}

void intializeDisplay(TwoWire *wireInterfaceDisplay, Adafruit_SSD1306 *display)
{
    wireInterfaceDisplay->begin(I2C_SDA, I2C_SCL, 100000);
    if (!display->begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        for (;;);
    }
    display->clearDisplay();
    display->setTextSize(2);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(22, 24);
    display->println("MediBoX");
    display->display();
    display->setTextSize(1);
    delay(350);
}
void loadAlarms(AlarmTime *alarms, Preferences *preferences)
{
    messageDisplay("Alarms", "Syncing..", syncing_alarms);
    for (int i = 0; i < 3; i++)
    {
        preferences->begin(alarms[i].alarm_name, false);
        Serial.println(alarms[i].alarm_name);
        alarms[i].hours = preferences->getUInt("hour", 0);
        alarms[i].minutes = preferences->getUInt("minute", 0);
        setAlarm(&alarms[i]);
        Serial.println("Alarm setted : " + String(alarms[i].hours) + ":" + String(alarms[i].minutes));
        preferences->end();
    }
    delay(350);
    messageDisplay("Alarms", "Synced !", syncing_alarms);
}

void saveAlarm(AlarmTime *alarm, Preferences *preferences)
{
    preferences->begin(alarm->alarm_name, false);
    preferences->putUInt("hour", alarm->hours);
    preferences->putUInt("minute", alarm->minutes);
    preferences->end();
}

void setAlarm(AlarmTime *alarm)
{
    long alarmTime;
    getTime(&timeinfo);
    long timeNowInSeconds = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;
    long alarmTimeInSeconds = (alarm->hours * 3600 + alarm->minutes * 60);
    Serial.println("Time now in seconds : " + String(timeNowInSeconds));
    if (alarmTimeInSeconds < timeNowInSeconds)
    {
        alarmTime = 86400 - timeNowInSeconds + alarmTimeInSeconds;
    }
    else
    {
        alarmTime = alarmTimeInSeconds - timeNowInSeconds;
    }
    alarm->alarmTimeInSeconds = alarmTime;
    alarm->isSet = true;
    Serial.println("Alarm time in seconds : " + String(alarm->alarmTimeInSeconds));
}
void disableAlarm(AlarmTime *alarm, Preferences *preferences)
{
    alarm->isSet = false;
    alarm->isRinging = false;
    saveAlarm(alarm, preferences);
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
void messageDisplay(String title, String Message, unsigned char bitmap[])
{
    byte title_legth = title.length();
    byte message_legth = Message.length();
    byte titleStartingX = (title_legth % 2 == 0) ? 64 - (title_legth / 2) * 6 : 64 - (title_legth / 2) * 6 - 3;
    byte messageStartingX = (message_legth % 2 == 0) ? 64 - (message_legth / 2) * 6 : 64 - (message_legth / 2) * 6 - 3;
    display.clearDisplay();
    display.setTextSize(1);
    display.drawBitmap(48, 16, bitmap, 32, 32, SSD1306_WHITE);
    display.setCursor(titleStartingX, 2);
    display.println(title);
    display.setCursor(messageStartingX, 52);
    display.println(Message);
    display.display();
}

void mainFaceDisplay(tm timeinfo, DHTData dhtData)
{
    display.clearDisplay();
    bool warn = handleWarning(dhtData);
    display.setCursor(0, 48);
    display.drawRect(12, 14, 104, 28, SSD1306_WHITE);
    display.setCursor(34, 0);
    display.println(&timeinfo, "%d-%m-%Y");
    display.setCursor(16, 20);
    display.setTextSize(2);
    display.println(&timeinfo, "%H:%M:%S");
    display.setTextSize(1);
    display.display();
    if(warn)
    {
        ledcWriteTone(0,3150);
        delay(200);
        ledcWriteTone(0,0);
    }
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
    saveAlarm(alarm, &preferences);
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
        menu.isSubMenuClosed = false;
        for (int i = 0; i < 3; i++)
        {
            disableAlarm(&alarms[i], &preferences);
        }
    }
}
void DHTInit(DHT *dht)
{
    messageDisplay("DHT", "Initializing..", dht_initializing);
    dht->begin();
    delay(350);
    messageDisplay("DHT", "Initialized", dht_initializing);
    delay(350);
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


void handleAlarmRinging(Button *cancelButton, Button *goForwardButton, Button *goBackwardButton, bool *ringingAlarm, long started)
{
    while (*ringingAlarm)
    {
        ledcWriteTone(0,880);
        delay(100);
        messageDisplay("Alarm", "Take your Medicine..", takeMedicine);
        pooling(goForwardButton, goBackwardButton, cancelButton);
        if (cancelButton->pressed)
        {
            *ringingAlarm = false;
            cancelButton->pressed = false;
            cancelButton->numberKeyPresses = 0;
            goForwardButton->pressed = false;
            goForwardButton->numberKeyPresses = 0;
            goBackwardButton->pressed = false;
            goBackwardButton->numberKeyPresses = 0;
            ledcWriteTone(0,0);
            break;
        }
        else if (millis() - started > 60000)
        {
            *ringingAlarm = false;
            ledcWriteTone(0,0);
            break;
        }
    }
}
bool handleWarning(DHTData dhtData)
{
    bool warn = false;
    if (dhtData.temperature >= TEMPURATION_UPPER_LIMIT || dhtData.temperature <= TEMPURATION_LOWER_LIMIT)
    {
        display.setCursor(0, 48);
        display.println("Temperature: " + String(dhtData.temperature) + "C");
        warn = true;
    }
    if (dhtData.humidity >= HUMIDITY_UPPER_LIMIT || dhtData.humidity <= HUMIDITY_LOWER_LIMIT)
    {
        display.setCursor(0, 56);
        display.println("Humidity: " + String(dhtData.humidity) + "%");
        warn = true;
    }
    if (dhtData.temperature > TEMPURATION_LOWER_LIMIT && dhtData.temperature < TEMPURATION_UPPER_LIMIT && dhtData.humidity > HUMIDITY_LOWER_LIMIT && dhtData.humidity < HUMIDITY_UPPER_LIMIT)
    {
        display.setCursor(31, 52);
        display.println("All Good :)");
    }
    return warn;

}

void setup()
{
    intializeDisplay(&wireInterfaceDisplay, &display);
    Serial.begin(112500);
    pinMode(LED, OUTPUT);
    ledcAttachPin(BUZZER, 0);
    pinMode(MenuInterruptPin, INPUT_PULLUP);
    pinMode(CancelInterruptPin, INPUT);
    pinMode(GoForwardInterruptPin, INPUT);
    pinMode(GoBackwardInterruptPin, INPUT);
    attachInterrupt(MenuInterruptPin, menuISR, RISING);
    wifiInit(&display);
    DHTInit(&dht);
    setTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    getTime(&timeinfo);
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    loadAlarms(alarms, &preferences);
    esp_sleep_enable_timer_wakeup(1 * 1000000); // light sleep for 2 seconds
    gpio_wakeup_enable(GPIO_NUM_27, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();
    esp_light_sleep_start();
}

void loop()
{
    long start = millis();
    measureDHT(&dhtData);
    getTime(&timeinfo);
    isAlarm(alarms);
    mainFaceDisplay(timeinfo,dhtData);
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
    handleAlarmRinging(&cancelButton, &goForwardButton, &goBackwardButton, &ringingAlarm, start);
    long end = millis();
    long timeElapsed = end - start;
    Serial.println("Time elapsed : " + String(timeElapsed) + " ms");
    esp_sleep_enable_timer_wakeup(1 * 1000000 - timeElapsed * 1000); // light sleep for 2 seconds
    esp_light_sleep_start();
}
