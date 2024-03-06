#include <Arduino.h>
#include <Preferences.h>

Preferences preferences;
void incrementAlarm(char *alarmName,int hour,int minute);
void setup()
{
    Serial.begin(115200);
    incrementAlarm("alarm1", 1, 30);

}


void loop()
{
}


void incrementAlarm(char *alarmName,int hour,int minute)
{
    preferences.begin(alarmName, false);
    int hours = preferences.getUInt("hour", 0);
    int minutes = preferences.getUInt("minute", 0);
    hours += hour;
    minutes += minute;
    preferences.putUInt("hour", hours);
    preferences.putUInt("minute", minutes);
    preferences.end();
}