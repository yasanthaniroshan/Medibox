#include <Arduino.h>


//Define the configurations for the OLED display
#define SCREEN_WIDTH 128
#define SCREEN_HIEGHT 64
#define OLED_RESET 4
#define SCREEN_ADDRESS 0x3C

// Define the configurations for the LED and BUZZER
#define LED 2
#define BUZZER 5


// Define the configurations for the buttons
#define MenuInterruptPin 27
#define GoForwardInterruptPin 26
#define GoBackwardInterruptPin 25
#define CancelInterruptPin 35


// Define the configurations for the I2C
#define I2C_SDA 33
#define I2C_SCL 32

// Define the configurations for the DHT sensor
#define DHTPIN 15     
#define DHTTYPE DHT22 


// Define the configurations Lower and Upper limits for the temperature 
#define TEMPURATION_LOWER_LIMIT 24
#define TEMPURATION_UPPER_LIMIT 32


// Define the configurations Lower and Upper limits for the humidity
#define HUMIDITY_LOWER_LIMIT 40
#define HUMIDITY_UPPER_LIMIT 80


// Define the structure to store the DHT sensor data
typedef struct
{
    float temperature;
    float humidity;
} DHTData;

// Define the structure to handle the button press event
typedef struct
{
    bool pressed;
    int millisPressed;
    int numberKeyPresses;
} Button;


// Define the structure to handle the menu
typedef struct
{
    bool pressed;
    bool isPopedUp;
    bool isClosed;
    bool isSubMenuClosed;
    int numberKeyPresses;

} Menu;


// Define the structure to handle the Gui Components
typedef struct
{
    void *selectedOption(void);
    int frameStartY;
} SelectedFrame;

// Define the structure to store and handle the time
typedef struct
{
    int time_id;
    const char *time_name;
    int hours;
    int minutes;
} Time;


// Define the structure to store and handle the alarm
typedef struct 
{
    Time *time;
    bool isSet;
    bool isRinging;
    int64_t alarmTimeInSeconds;
}Alarm;

// Define the Enuemration for the Menu Options
typedef enum
{
    Alarm_01 = 0,
    Alarm_02 = 1,
    Alarm_03 = 2,
    SetOffset = 3,
    DisableAllAlarms = 4
} MenuOptions;
