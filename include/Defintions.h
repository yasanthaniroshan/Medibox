#include <Arduino.h>


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
    int numberKeyPresses;

} Menu;

typedef struct
{
    void *selectedOption(void);
    int frameStartY;
} SelectedFrame;
typedef struct
{
    int time_id;
    const char *time_name;
    int hours;
    int minutes;
} Time;

typedef struct 
{
    Time *time;
    bool isSet;
    bool isRinging;
    int64_t alarmTimeInSeconds;
}Alarm;




typedef enum
{
    Alarm_01 = 0,
    Alarm_02 = 1,
    Alarm_03 = 2,
    SetOffset = 3,
    DisableAllAlarms = 4
} MenuOptions;
