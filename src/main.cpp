#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include "time.h"
#include <Preferences.h>
#include <Bitmaps.h>
#include <Defintions.h>

// Initialize the MenuOptions Structure to store the menu options (Default is Alarm_01)
MenuOptions selectedOption = Alarm_01;

// Initialize the SelectedFrame Structure
SelectedFrame selectedFrame = {10};

// Initialize the Button Structure to store the button press event
Button menuButton = {false, 0, 0};
Button goForwardButton = {false, 0, 0};
Button goBackwardButton = {false, 0, 0};
Button cancelButton = {false, 0, 0};

// Initialize the Menu Structure to store the menu state variables
Menu menu = {false, false, false, false, 0};

// Initialize the Time Structure to store the time
Time time_01 = {0, "Alarm-01", 0, 0};
Time time_02 = {1, "Alarm-02", 0, 0};
Time time_03 = {2, "Alarm-03", 0, 0};

// Initialize the Alarm Structure to store the alarm state variables
Alarm alarms[3] = {{&time_01, false, false, 0}, {&time_02, false, false, 0}, {&time_03, false, false, 0}};

// Initialize the Alarm Pointer Array to store the pointers to the alarms
Alarm *alarmPointers[3] = {&alarms[0], &alarms[1], &alarms[2]};

// Initialize the Time Structure to store the time offset from UTC
Time offset = {3, "Offset", 5, 30};

// Set ssid and password for the wifi
const char *ssid = "MSI 8690";
const char *password = "abcdefgh";

// Set the NTP Server and the GMT and Daylight offset
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

// Declare global variable ringingAlarm to store the state of the alarm
bool ringingAlarm = false;

// Function Declarations


void loadOffsets(Time *offset, Preferences *preferences);
void saveOffset(Time *offset, Preferences *preferences);
bool handleWarning(DHTData dhtData);
void intializeDisplay(TwoWire *wireInterfaceDisplay, Adafruit_SSD1306 *display);
void messageDisplay(String title, String Message, unsigned char bitmap[]);
void DHTInit(DHT *dht);
void wifiInit(Adafruit_SSD1306 *display);
void ringAlarm();
void measureDHT(DHTData *dhtData);
void setTime(Time offset, int daylightOffset_sec, const char *ntpServer);
void intializeDisplay();
void loadAlarms(Alarm **alarms[], Preferences *preferences);
void saveAlarm(Alarm *alarm, Preferences *preferences);
void setAlarm(Alarm *alarm);
void changeTimeZone(Time *time, Preferences *preferences);
void displayText(String text);
void mainFaceDisplay(tm timeinfo, DHTData dhtData);
void displayAlarm(byte data, String text);
void handleAlarm(Alarm *alarm, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton);
void handleTimeZone(Alarm *time, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton);
void adjustAlarmHours(Alarm *alarm, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton);
void adjustAlarmMinutes(Alarm *alarm, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton);
void adjustTimeZoneHours(Time *time, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton);
void adjustTimeZoneMinutes(Time *time, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton);
void measureDHT(DHTData *dhtData);
void getTime(tm *timeinfo);
bool debounce(Button *button);
void displayMenu(SelectedFrame *selectedFrame, Menu *menu);
void pooling(Button *goForwardButton, Button *goBackwardButton, Button *cancelButton);
void handleMenu(Alarm *alarms[], Time *offset, MenuOptions selectedOption, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton);
bool handleMainMenu(Alarm *alarms[], Time *offset, SelectedFrame *selectedFrame, Menu *menu, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton);

// Initialize the time structure to store the time
struct tm timeinfo;

// Initialize the DHTData structure to store the DHT sensor data
DHTData dhtData;

// Initialize the Preferences structure to store the alarm data
Preferences preferences;

// Define the I2C pins for the OLED Display
TwoWire wireInterfaceDisplay = TwoWire(0);

// Initialize the OLED Display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HIEGHT, &wireInterfaceDisplay, OLED_RESET);

// Initialize the DHT sensor object
DHT dht(DHTPIN, DHTTYPE);

/**
 * @brief
 * Checks if any of the alarms in the given array should ring at the current time.
 * If an alarm should ring, it sets the corresponding alarm's `isRinging` flag to true,
 * sets the global `ringingAlarm` flag to true, and marks the alarm as not set.
 * If an alarm is set but its time has not yet arrived, it decrements the alarm's
 * `alarmTimeInSeconds` by 1.
 *
 * @param alarms An array of pointers to Alarm objects.
 *
 * @return void
 */
void isAlarm(Alarm *alarms[])
{
    getTime(&timeinfo);
    for (int i = 0; i < 3; i++)
    {

        if (alarms[i]->isSet && timeinfo.tm_hour == alarms[i]->time->hours && timeinfo.tm_min == alarms[i]->time->minutes)
        {
            alarms[i]->isRinging = true;
            ringingAlarm = true;
            alarms[i]->isSet = false;
        }
        else if (alarms[i]->isSet && alarms[i]->alarmTimeInSeconds > 0)
        {
            alarms[i]->alarmTimeInSeconds--;
        }
    }
}
/**
 * @brief Initializes the WiFi connection and prints the status to the display.
 *
 * @param display A pointer to the Adafruit_SSD1306 display object.
 *
 * @return void
 */
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
/**
 * @brief Configures the time settings for ESP32 using the given GMT offset, daylight offset, and NTP server.
 *
 * @param gmtOffset_sec The GMT offset in seconds.
 * @param daylightOffset_sec The daylight offset in seconds.
 * @param ntpServer The NTP server to use for time synchronization.
 *
 * @return void
 */
void setTime(Time offset, int daylightOffset_sec, const char *ntpServer)
{

    int offsetInSeconds = offset.hours * 3600 + offset.minutes * 60;
    messageDisplay("Time", "Configuring..", timeConfig);
    configTime(offsetInSeconds, daylightOffset_sec, ntpServer);
    delay(350);
    messageDisplay("Time", "Configured", timeConfig);
}
/**
 * @brief Initializes the DHT sensor.
 *
 * @param wireInterfaceDisplay Pointer to the TwoWire interface for the display.
 * @param display Pointer to the Adafruit_SSD1306 display object.
 *
 * @return void
 */
void intializeDisplay(TwoWire *wireInterfaceDisplay, Adafruit_SSD1306 *display)
{

    wireInterfaceDisplay->begin(I2C_SDA, I2C_SCL, 100000);
    if (!display->begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        for (;;)
            ;
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
/**
 * @brief Loads alarms from non-volatile memory and sets them.
 *
 * This function loads alarms from memory and sets them using the provided Alarm and Preferences objects.
 * It syncs the alarms and displays a message during the syncing process.
 *
 * @param alarms An array of Alarm pointers representing the alarms to be loaded and set.
 * @param preferences A pointer to the Preferences object used to access the alarm details.
 *
 * @return void
 */
void loadAlarms(Alarm *alarms[], Preferences *preferences)
{
    messageDisplay("Alarms", "Syncing..", syncing_alarms);
    for (int i = 0; i < 3; ++i)
    {
        preferences->begin(alarms[i]->time->time_name, false);
        alarms[i]->time->hours = preferences->getUInt("hour", 0);
        alarms[i]->time->minutes = preferences->getUInt("minute", 0);
        setAlarm(alarms[i]);
        preferences->end();
    }
    delay(350);
    messageDisplay("Alarms", "Synced !", syncing_alarms);
}
/**
 * Loads the offset values from the preferences and assigns them to the given Time object.
 *
 * @param offset A pointer to the Time object to store the offset values.
 * @param preferences A pointer to the Preferences object to retrieve the offset values from.
 */
void loadOffsets(Time *offset, Preferences *preferences)
{
    preferences->begin(offset->time_name, false);
    offset->hours = preferences->getUInt("hour", offset->hours);
    offset->minutes = preferences->getUInt("minute", offset->minutes);
    preferences->end();
}
/**
 * @brief Saves the alarm details to the non-volatile memory.
 *
 * @param alarm The alarm object containing the alarm details.
 * @param preferences The preferences object used to store the alarm details.
 *
 * @return void
 */
void saveAlarm(Alarm *alarm, Preferences *preferences)
{

    preferences->begin(alarm->time->time_name, false);
    preferences->putUInt("hour", alarm->time->hours);
    preferences->putUInt("minute", alarm->time->minutes);
    preferences->end();
}

/**
 * Saves the offset time to the preferences.
 * 
 * @param offset - Pointer to the Time object representing the offset.
 * @param preferences - Pointer to the Preferences object for storing the offset.
 */
void saveOffset(Time *offset, Preferences *preferences)
{
    preferences->begin(offset->time_name, false);
    preferences->putUInt("hour", offset->hours);
    preferences->putUInt("minute", offset->minutes);
    preferences->end();
}


/**
 * @brief Sets the alarm time and marks the alarm as set.
 *
 * This function sets the alarm time and marks the alarm as set. while calculating the time remaining for the alarm to ring.
 *
 * @param alarm A pointer to the Alarm object.
 *
 * @return void
 */
void setAlarm(Alarm *alarm)
{
    long alarmTime;
    getTime(&timeinfo);
    long timeNowInSeconds = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;
    long alarmTimeInSeconds = (alarm->time->hours * 3600 + alarm->time->minutes * 60);
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
}
/**
 * @brief Disables the alarm by setting its `isSet` and `isRinging` flags to false.
 *
 * Saves the updated alarm state using the provided preferences.
 *
 * @param alarm The alarm to be disabled.
 * @param preferences The preferences object used to save the updated alarm state.
 *
 * @return void
 */
void disableAlarm(Alarm *alarm, Preferences *preferences)
{

    alarm->isSet = false;
    alarm->isRinging = false;
    saveAlarm(alarm, preferences);
}

/**
 * @brief Changes the time zone of the given Time object.
 * 
 * The function calculates the offset based on the hours and minutes of the Time object,
 * and then calls the `configTime` function to update the time zone.
 *
 * @param time A pointer to the Time object whose time zone needs to be changed.
 * 
 * @return void
 */
void changeTimeZone(Time *time, Preferences *preferences)
{
    int offset;
    if (time->hours > -12 && time->hours < 0)
    {
        offset = time->hours * 3600 - time->minutes * 60;
    }
    else if (time->hours > 0 && time->hours < 14)
    {
        offset = time->hours * 3600 + time->minutes * 60;
    }
    else if (time->hours == -12)
    {
        offset = time->hours * 3600;
    }
    else if (time->hours == 14)
    {
        offset = time->hours * 3600;
    }
    saveOffset(time, preferences);
    configTime(offset, 0, ntpServer);
}

/**
 * 
 * @brief Displays a message on the screen with a given title, message, and bitmap.
 * 
 * @param title The title of the message.
 * @param Message The message to be displayed.
 * @param bitmap An array of unsigned characters representing the bitmap to be displayed.
 * 
 * @return void
 */
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

/**
 * @brief Displays the main face of the device with the current time and warning status.
 * 
 * This function clears the display and then displays the current date and time on the OLED display.
 * It also checks for any warning conditions based on the DHTData and displays a warning message if necessary.
 * If a warning condition is detected, it activates an LED tone for a short duration.
 * 
 * @param timeinfo The current time information.
 * @param dhtData The data from the DHT sensor.
 * 
 * @return void
 */
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
    if (warn)
    {
        ledcWriteTone(0, 3150);
        delay(200);
        ledcWriteTone(0, 0);
    }
}

/**
 * @brief Displays the given text on the OLED display.
 * 
 * @param data The alarm data to be displayed.
 * @param text The text to be displayed along with the alarm.
 * 
 * @return void
 */
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
/**
 * @brief Handles the alarm configuration process.
 * 
 * This function handles the alarm configuration process by displaying the current alarm time and allowing the user to adjust it.
 * 
 * @param alarm The alarm object to be configured.
 * @param menuButton The button object used to navigate the menu.
 * @param goForwardButton The button object used to move forward in the menu.
 * @param goBackwardButton The button object used to move backward in the menu.
 * @param cancelButton The button object used to cancel the configuration process.
 * 
 * @return void
 */
void handleAlarm(Alarm *alarm, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton)
{
    displayAlarm(alarm->time->hours, "Hours");
    while (!menu.isSubMenuClosed)
    {
        adjustAlarmHours(alarm, menuButton, goForwardButton, goBackwardButton, cancelButton);
    }
    menu.isSubMenuClosed = false;
    displayAlarm(alarm->time->minutes, "Minutes");
    while (!menu.isSubMenuClosed)
    {
        adjustAlarmMinutes(alarm, menuButton, goForwardButton, goBackwardButton, cancelButton);
    }
    menu.isSubMenuClosed = true;
    setAlarm(alarm);
    saveAlarm(alarm, &preferences);
}

/**
 * @brief Handles the time zone configuration process.
 * 
 * This function handles the time zone configuration process by displaying the current time zone and allowing the user to adjust it.
 * 
 * @param time The time object to be configured.
 * @param menuButton The button object used to navigate the menu.
 * @param goForwardButton The button object used to move forward in the menu.
 * @param goBackwardButton The button object used to move backward in the menu.
 * @param cancelButton The button object used to cancel the configuration process.
 * 
 * @return void
 */
void handleTimeZone(Time *time, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton)
{
    displayAlarm(time->hours, "Hours");
    while (!menu.isSubMenuClosed)
    {
        adjustTimeZoneHours(time, menuButton, goForwardButton, goBackwardButton, cancelButton);
    }
    menu.isSubMenuClosed = false;
    displayAlarm(time->minutes, "Minutes");
    while (!menu.isSubMenuClosed)
    {
        adjustTimeZoneMinutes(time, menuButton, goForwardButton, goBackwardButton, cancelButton);
    }
    menu.isSubMenuClosed = true;
    changeTimeZone(time, &preferences);
}

/**
 * @brief Adjusts the alarm hours based on the button press event.
 * 
 * This function adjusts the alarm hours based on the button press event.
 * 
 * @param alarm The alarm object to be configured.
 * @param menuButton The button object used to navigate the menu.
 * @param goForwardButton The button object used to increase the hours.
 * @param goBackwardButton The button object used to decrease the hours.
 * @param cancelButton The button object used to exit from the editing
 * 
 * @return void
 */
void adjustAlarmHours(Alarm *alarm, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton)
{
    pooling(goForwardButton, goBackwardButton, cancelButton);
    if (goForwardButton->pressed)
    {
        if (alarm->time->hours < 23)
        {
            alarm->time->hours++;
        }
        else
        {
            alarm->time->hours = 0;
        }
        goForwardButton->pressed = false;
        goForwardButton->numberKeyPresses = 0;
        displayAlarm(alarm->time->hours, "Hours");
    }
    else if (goBackwardButton->pressed)
    {
        if (alarm->time->hours > 0)
        {
            alarm->time->hours--;
        }
        else
        {
            alarm->time->hours = 23;
        }
        goBackwardButton->pressed = false;
        goBackwardButton->numberKeyPresses = 0;
        displayAlarm(alarm->time->hours, "Hours");
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


/**
 * @brief Adjusts the alarm minutes based on the button presses.
 *  
 * @param alarm The alarm object to adjust.
 * @param menuButton The menu button object.
 * @param goForwardButton The go forward button object.
 * @param goBackwardButton The go backward button object.
 * @param cancelButton The cancel button object.
 * 
 * @return void
 */
void adjustAlarmMinutes(Alarm *alarm, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton)
{
    pooling(goForwardButton, goBackwardButton, cancelButton);
    if (goForwardButton->pressed)
    {
        if (alarm->time->minutes < 59)
        {
            alarm->time->minutes++;
        }
        else
        {
            alarm->time->minutes = 0;
        }
        goForwardButton->pressed = false;
        goForwardButton->numberKeyPresses = 0;
        displayAlarm(alarm->time->minutes, "Minutes");
    }
    else if (goBackwardButton->pressed)
    {
        if (alarm->time->minutes > 0)
        {
            alarm->time->minutes--;
        }
        else
        {
            alarm->time->minutes = 59;
        }
        goBackwardButton->pressed = false;
        goBackwardButton->numberKeyPresses = 0;
        displayAlarm(alarm->time->minutes, "Minutes");
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
/**
 * @brief Adjusts the time zone hours based on the button inputs.
 * 
 * @param time              A pointer to the Time object representing the current time.
 * @param menuButton        A pointer to the Menu Button object.
 * @param goForwardButton   A pointer to the Go Forward Button object.
 * @param goBackwardButton  A pointer to the Go Backward Button object.
 * @param cancelButton     A pointer to the Cancel Button object.
 * 
 * @return void
 */
void adjustTimeZoneHours(Time *time, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton)
{
    pooling(goForwardButton, goBackwardButton, cancelButton);
    if (goForwardButton->pressed)
    {
        if (time->hours > -13 && time->hours < 15)
        {
            time->hours++;
        }
        else if (time->hours >= 15)
        {
            time->hours = -13;
        }
        else if (time->hours <= -13)
        {
            time->hours = 15;
        }
        goForwardButton->pressed = false;
        goForwardButton->numberKeyPresses = 0;
        displayAlarm(time->hours, "Hours");
    }
    else if (goBackwardButton->pressed)
    {
        if (time->hours > -13 && time->hours < 15)
        {
            time->hours--;
        }
        else if (time->hours <= -13)
        {
            time->hours = 15;
        }
        else if (time->hours >= 15)
        {
            time->hours = -13;
        }
        goBackwardButton->pressed = false;
        goBackwardButton->numberKeyPresses = 0;
        displayAlarm(time->hours, "Hours");
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
/**
 * @brief Adjusts the time zone minutes based on the button inputs.
 * 
 * @param time              A pointer to the Time object representing the current time.
 * @param menuButton        A pointer to the Menu Button object.
 * @param goForwardButton   A pointer to the Go Forward Button object.
 * @param goBackwardButton  A pointer to the Go Backward Button object.
 * @param cancelButton     A pointer to the Cancel Button object.
 * 
 * @return void
 */
void adjustTimeZoneMinutes(Time *time, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton)
{
    pooling(goForwardButton, goBackwardButton, cancelButton);
    if (goForwardButton->pressed)
    {
        if (time->minutes < 59)
        {
            time->minutes++;
        }
        else
        {
            time->minutes = 0;
        }
        goForwardButton->pressed = false;
        goForwardButton->numberKeyPresses = 0;
        displayAlarm(time->minutes, "Minutes");
    }
    else if (goBackwardButton->pressed)
    {
        if (time->minutes > 0)
        {
            time->minutes--;
        }
        else
        {
            time->minutes = 59;
        }
        goBackwardButton->pressed = false;
        goBackwardButton->numberKeyPresses = 0;
        displayAlarm(time->minutes, "Minutes");
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

/**
 * @brief Handles the menu selection and navigation.
 * 
 * This function handles the menu selection and navigation based on the button press events.
 * 
 * @param alarms An array of pointers to Alarm objects.
 * @param offset A pointer to the Time object representing the time offset from UTC.
 * @param selectedOption The selected menu option.
 * @param menuButton The button object used to navigate the menu.
 * @param goForwardButton The button object used to move forward in the menu.
 * @param goBackwardButton The button object used to move backward in the menu.
 * @param cancelButton The button object used to cancel the menu selection.
 * 
 * @return void
 */
void handleMenu(Alarm *alarms[], Time *offset, MenuOptions selectedOption, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton)
{
    if (selectedOption == Alarm_01 || selectedOption == Alarm_02 || selectedOption == Alarm_03)
    {
        menu.isSubMenuClosed = false;
        handleAlarm(alarms[selectedOption], menuButton, goForwardButton, goBackwardButton, cancelButton);
        menu.isClosed = true;
        messageDisplay("Alarm", "Alarm set !", alarm_success);
        delay(600);
        return;
    }
    else if (selectedOption == SetOffset)
    {
        menu.isSubMenuClosed = false;
        handleTimeZone(offset, menuButton, goForwardButton, goBackwardButton, cancelButton);
        messageDisplay("Setting Offset", "Offset Changed", Offset_set);
        delay(600);
        menu.isClosed = true;
    }
    else if (selectedOption == DisableAllAlarms)
    {
        menu.isSubMenuClosed = false;
        for (int i = 0; i < 3; i++)
        {
            messageDisplay("Alarms", "Disabling Alarms", disable_alarms);
            disableAlarm(alarms[i], &preferences);
            delay(600);
        }
        menu.isClosed = true;
    }
}

/**
 * @brief Initializes the DHT sensor.
 * 
 * @param dht A pointer to the DHT sensor object.
 * 
 * @return void
 */
void DHTInit(DHT *dht)
{
    messageDisplay("DHT", "Initializing..", dht_initializing);
    dht->begin();
    delay(350);
    messageDisplay("DHT", "Initialized", dht_initializing);
    delay(350);
}

/**
 * @brief Measures the temperature and humidity using the DHT sensor.
 * 
 * This function measures the temperature and humidity using the DHT sensor and stores the values in the given DHTData object.
 * 
 * @param dhtData A pointer to the DHTData object to store the temperature and humidity values.
 * 
 * @return void
 */


void measureDHT(DHTData *dhtData)
{

    dhtData->temperature = dht.readTemperature();
    dhtData->humidity = dht.readHumidity();

    if (isnan(dhtData->temperature) || isnan(dhtData->humidity))
    {
        return;
    }
}

/**
 * @brief Gets the current time and stores it in the given timeinfo structure.
 * 
 * @param timeinfo A pointer to the tm structure to store the current time.
 * 
 * @return void
 */
void getTime(tm *timeinfo)
{
    if (!getLocalTime(timeinfo))
    {
        return;
    }
}


/**
 * @brief Interrupt service routine for the menu button.
 * 
 * This function is called when the menu button interrupt is triggered.
 * It updates the `millisPressed` variable of the `menuButton` object with the current millis() value,
 * and sets the `pressed` flag to true.
 * 
 * @note This function is marked with the `IRAM_ATTR` attribute to ensure that it is placed in the
 * IRAM (instruction RAM) section of the memory, which allows for faster execution.
 * 
 * @return void
 */
void IRAM_ATTR menuISR()
{
    menuButton.millisPressed = millis();
    menuButton.pressed = true;
}

/**
 * @brief Checks if a button press is debounced.
 * 
 * @param button A pointer to the Button object to check.
 * @return True if the button press is debounced, false otherwise.
 */
bool debounce(Button *button)
{
    if (button->pressed && (millis() - button->millisPressed) > 100)
    {
        return true;
    }
    return false;
}

/**
 * @brief Displays the menu on the OLED display.
 * 
 * @param selectedFrame A pointer to the SelectedFrame object representing the selected frame.
 * @param menu A pointer to the Menu object representing the menu state.
 * 
 * @return void
 */
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

/**
 * @brief Pools the button inputs and updates the button state.
 * 
 * This function pools the button inputs and updates the button state based on the input.
 * 
 * @param goForwardButton A pointer to the Go Forward Button object.
 * @param goBackwardButton A pointer to the Go Backward Button object.
 * @param cancelButton A pointer to the Cancel Button object.
 * 
 * @return void
 */

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

/**
 * @brief Handles the main menu navigation and selection.
 * 
 * This function handles the main menu navigation and selection based on the button press events.
 * 
 * @param alarms An array of pointers to Alarm objects.
 * @param offset A pointer to the Time object representing the time offset from UTC.
 * @param selectedFrame A pointer to the SelectedFrame object representing the selected frame.
 * @param menu A pointer to the Menu object representing the menu state.
 * @param menuButton The button object used to navigate the menu.
 * @param goForwardButton The button object used to move forward in the menu.
 * @param goBackwardButton The button object used to move backward in the menu.
 * @param cancelButton The button object used to cancel the menu selection.
 * 
 * @return True if the main menu is closed, false otherwise.
 */

bool handleMainMenu(Alarm *alarms[], Time *offset, SelectedFrame *selectedFrame, Menu *menu, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton)
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
        handleMenu(alarms, offset, selectedOption, menuButton, goForwardButton, goBackwardButton, cancelButton);
        return true;
    }
    return false;
}


/**
 * @brief Handles the alarm ringing process.
 * 
 * This function handles the alarm ringing process by displaying the alarm message and allowing the user to cancel the alarm.
 * 
 * @param cancelButton The button object used to cancel the alarm.
 * @param goForwardButton The button object used to move forward in the menu.
 * @param goBackwardButton The button object used to move backward in the menu.
 * @param ringingAlarm The flag indicating if an alarm is ringing.
 * @param started The time when the alarm started ringing.
 * 
 * @return void
 */
void handleAlarmRinging(Button *cancelButton, Button *goForwardButton, Button *goBackwardButton, bool *ringingAlarm, long started)
{
    while (*ringingAlarm)
    {
        ledcWriteTone(0, 880);
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
            ledcWriteTone(0, 0);
            break;
        }
        else if (millis() - started > 60000)
        {
            *ringingAlarm = false;
            ledcWriteTone(0, 0);
            break;
        }
    }
}

/**
 * @brief Handles the warning conditions based on the DHT sensor data.
 * 
 * This function checks for any warning conditions based on the DHT sensor data and displays a warning message if necessary.
 * 
 * @param dhtData A pointer to the DHTData object representing the DHT sensor data.
 * 
 * @return True if a warning condition is detected, false otherwise.
 */
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

    // Initialize the display
    intializeDisplay(&wireInterfaceDisplay, &display);

    //Define the pins for the buttons and the buzzer
    pinMode(LED, OUTPUT);
    ledcAttachPin(BUZZER, 0);
    pinMode(MenuInterruptPin, INPUT_PULLUP);
    pinMode(CancelInterruptPin, INPUT);
    pinMode(GoForwardInterruptPin, INPUT);
    pinMode(GoBackwardInterruptPin, INPUT);

    // Attach the interrupt service routine for the menu button
    attachInterrupt(MenuInterruptPin, menuISR, RISING);

    // Initialize the WiFi connection
    wifiInit(&display);

    // Initialize the DHT sensor
    DHTInit(&dht);

    // Load the offsets from non-volatile memory
    loadOffsets(&offset, &preferences);

    // Set the time zone and configure the time settings using the NTP server
    setTime(offset, daylightOffset_sec, ntpServer);

    // Load the alarms from RTC and store in timeinfo
    getTime(&timeinfo);

    // Load the alarms from non-volatile memory
    loadAlarms(alarmPointers, &preferences);

    // Set the wake up time for the ESP32 using timer
    esp_sleep_enable_timer_wakeup(1 * 1000000);

    // Set the wake up precedure for the ESP32 using GPIO
    gpio_wakeup_enable(GPIO_NUM_27, GPIO_INTR_LOW_LEVEL);

    // Enable the GPIO wake up
    esp_sleep_enable_gpio_wakeup();

    // Enter the light sleep mode
    esp_light_sleep_start();
}

void loop()
{

    //get the current time since device booted up 
    long start = millis();

    // Measure the temperature and humidity using the DHT sensor
    measureDHT(&dhtData);

    // Get the current time stored in the timeinfo structure
    getTime(&timeinfo);

    // Check if any of the alarms should ring
    isAlarm(alarmPointers);

    // Display the main face of the device
    mainFaceDisplay(timeinfo, dhtData);

    // Handle the menu button press event
    if (menuButton.pressed && debounce(&menuButton))
    {
        // Set the menu button state to false
        menuButton.pressed = false;

        // Display the menu on the OLED display
        displayMenu(&selectedFrame, &menu);

        // wait for the menu to be closed
        while (!menu.isClosed)
        {
            //Handle the main menu navigation and selection
            bool responce = handleMainMenu(alarmPointers, &offset, &selectedFrame, &menu, &menuButton, &goForwardButton, &goBackwardButton, &cancelButton);
        }

        // Set the menu state to false
        menu.isClosed = false;
    }

    // Handle the alarm ringing process
    handleAlarmRinging(&cancelButton, &goForwardButton, &goBackwardButton, &ringingAlarm, start);

    // get the time elapsed since the device booted up
    long end = millis();

    // calculate the time elapsed
    long timeElapsed = end - start;

    // Set the wake up time for the ESP32 using timer
    esp_sleep_enable_timer_wakeup(1 * 1000000 - timeElapsed * 1000);

    // Enter the light sleep mode
    esp_light_sleep_start();
}
