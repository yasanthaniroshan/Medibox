#include <Arduino.h>


//Define the configurations for the OLED display
#define SCREEN_WIDTH 128
#define SCREEN_HIEGHT 64
#define OLED_RESET 4
#define SCREEN_ADDRESS 0x3C

// Define the configurations for the LED and BUZZER
#define LED 2
#define BUZZER 25

// Define the configurations for the LDR sensors
#define LDR_LEFT_PIN 35
#define LDR_RIGHT_PIN 34

// Define the configurations for the Servo Motor
#define SERVO_MOTOR_PIN 33


// Define the configurations for the buttons
#define MenuInterruptPin 23
#define GoForwardInterruptPin 5
#define GoBackwardInterruptPin 18
#define CancelInterruptPin 19


// Define the configurations for the I2C
#define I2C_SDA 21
#define I2C_SCL 22

// Define the configurations for the DHT sensor
#define DHTPIN 26     
#define DHTTYPE DHT22 


// Define the configurations Lower and Upper limits for the temperature 
#define TEMPURATION_LOWER_LIMIT 27
#define TEMPURATION_UPPER_LIMIT 35


// Define the configurations Lower and Upper limits for the humidity
#define HUMIDITY_LOWER_LIMIT 40
#define HUMIDITY_UPPER_LIMIT 90


// Define the configurations for the Light Intensity Calculation
#define GAMMA 0.7
#define RL10 50


// Define the configurations for the MQTT Broker
#define MQTT_SERVER "r1acdb31.ala.asia-southeast1.emqxsl.com"
#define MQTT_PORT 8883
#define MQTT_CLIENT_ID "ESP32-0101"




// Define the configurations for the MQTT Topics
#define LDR_TOPIC "LDR"
#define DHT_DATA_TOPIC "DHT_DATA"
#define CONFIGURATION_TOPIC "CONFIGURATION"
#define SERVO_TOPIC "SERVO"


// Define the structure to store the DHT sensor data
typedef struct
{
    float temperature;
    float humidity;
} DHTData;


// Define the structure to store the LDR sensor data
typedef struct 
{
    float leftLDRValue;
    float rightLDRValue;
    float maximumValue;
    bool isLeftLDRHigh;
} LDRValue;


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
