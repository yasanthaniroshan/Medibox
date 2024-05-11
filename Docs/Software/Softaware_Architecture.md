# Software Architecture Document
## Introduction

This document describes the software architecture of the MediBox  system. MediBox is a smart pharmaceutical storage system designed to manage medications effectively. The system leverages an ESP32 microcontroller, integrated sensors (DHT sensor, LDRs), actuators (buzzer, servo motor), and an OLED display for user interaction. The software interacts with a remote MQTT broker and Node-RED dashboard for data visualization and remote control.

## Architectural Overview

The MediBox software architecture follows a modular design, dividing the system into distinct functional components
- Hardware Abstraction Layer : This layer encapsulates the hardware-specific functionalities, providing an abstract interface for interacting with the ESP32 peripherals (OLED, buzzer, buttons, LDRs, servo motor).
- Sensor Management: This component handles data acquisition from the DHT sensor and LDRs, processing and storing the collected information.
- Alarm Management: This module manages the alarms, allowing users to set, disable, and be alerted by alarms using the buzzer.
- Time Management: This component handles time synchronization with an NTP server, incorporating user-defined time zone offsets.
- User Interface: This layer manages the user interaction with the system through the OLED display and buttons.
- Communication Management: This component handles communication with the MQTT broker, publishing sensor data and receiving control commands for the servo motor.
## Component Description

### Hardware Abstraction Layer

This layer provides an abstraction for interacting with the hardware, simplifying the software design and allowing for easy adaptation to different hardware configurations.

- OLED Display

    - Initialized using the Adafruit_SSD1306 library, configuring screen dimensions, reset pin, and I2C address:

        ```cpp
        TwoWire wireInterfaceDisplay = TwoWire(0); 
        Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HIEGHT, &wireInterfaceDisplay, OLED_RESET);
        ```

    - Functions for text manipulation, drawing shapes, and displaying bitmaps are provided by the library.

    - Example usage:

        ```cpp
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(22, 24);
        display.println("MediBoX");
        display.display();
        ```

- Buzzer:

    - Controlled using the ledcWriteTone function, which allows for generating tones at specific frequencies:

        ```cpp
        ledcAttachPin(BUZZER, 0); // Attach buzzer pin to LEDC channel 0
        ledcWriteTone(0, 880); // Generate a tone at 880 Hz
        delay(100); // Tone duration
        ledcWriteTone(0, 0); // Turn off the buzzer
        ```

- Buttons:

    - Pin modes are set as inputs with internal pull-up resistors:

        ```cpp
        pinMode(MenuInterruptPin, INPUT_PULLUP);
        ```

    - Interrupt handling is implemented for the menu button using attachInterrupt:

        ```cpp
        attachInterrupt(MenuInterruptPin, menuISR, RISING);
        ```

- LDRs:

    - Analog values are read using analogRead and converted to illuminance (lux) using a formula based on voltage and resistance:

        ```cpp
        float calculateLuxValue(float sensorValue) {
            float voltage = sensorValue / 4096.0 * 3.3;
            float resistance = 10000 * voltage / (3.3 - voltage);
            float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));
            if (lux >= 10000) return 1;
            return lux / 10000;
        }
        ```

- Servo Motor:

    - Controlled using the Servo library, allowing for setting the servo motor angle:

        ```cpp
        Servo servo;
        servo.attach(SERVO_MOTOR_PIN);
        servo.write(0); // Set the servo angle to 90 degrees
        ```

### Sensor Management

This component manages data acquisition from the sensors, ensuring that sensor readings are properly processed and utilized within the system.

- DHT Sensor:

    - The DHT library simplifies reading temperature and humidity data:

        ```cpp
        DHT dht(DHTPIN, DHTTYPE);
        dhtData.temperature = dht.readTemperature();
        dhtData.humidity = dht.readHumidity();
        ```


    - Warning conditions are checked against pre-defined thresholds, triggering alerts on the OLED and through the buzzer if necessary:

        ```cpp
        bool handleWarning(DHTData dhtData) {
            bool warn = false;
            if (dhtData.temperature >= TEMPURATION_UPPER_LIMIT || dhtData.temperature <= TEMPURATION_LOWER_LIMIT) {
                // Display warning on OLED
                warn = true; 
            }
            // Similar logic for humidity 
            return warn; 
        }
        ```


- LDRs:

    - Left and right LDR readings are compared to determine the maximum light intensity:

        ```cpp
        readLDRValues(&ldrvalue);
        if (ldrvalue.leftLDRValue > ldrvalue.rightLDRValue) {
            ldrvalue.maximumValue = roundf(ldrvalue.leftLDRValue * 1000.0) / 1000.0;
            ldrvalue.isLeftLDRHigh = true;
        } else if (ldrvalue.leftLDRValue <= ldrvalue.rightLDRValue) {
            ldrvalue.maximumValue = roundf(ldrvalue.rightLDRValue * 1000) / 1000.0;
            ldrvalue.isLeftLDRHigh = false;
        }
        ```

### Alarm Management

This module provides functionalities for setting, managing, and triggering alarms within the system.

- Alarm Configuration:

    - Allows users to set alarm times (hours and minutes) using the OLED interface.

    - Saves alarm configurations to non-volatile memory using the Preferences library:

        ```cpp
        Preferences preferences; 
        preferences.begin(alarm->time->time_name, false);
        preferences.putUInt("hour", alarm->time->hours);
        preferences.putUInt("minute", alarm->time->minutes);
        preferences.end();
        ```

    - Calculates the remaining time (in seconds) until an alarm triggers:

        ```cpp
        long timeNowInSeconds = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;
        long alarmTimeInSeconds = (alarm->time->hours * 3600 + alarm->time->minutes * 60);
        if (alarmTimeInSeconds < timeNowInSeconds) {
            alarmTime = 86400 - timeNowInSeconds + alarmTimeInSeconds; 
        } else {
            alarmTime = alarmTimeInSeconds - timeNowInSeconds; 
        }
        alarm->alarmTimeInSeconds = alarmTime;
        ```

- Alarm Triggering:

    - Continuously checks if the current time matches any set alarms.

    - Activates the buzzer using ledcWriteTone when an alarm triggers, generating a distinct alarm tone.

    - Displays an alarm notification on the OLED screen, reminding users to take their medication.

    - The isAlarm function iterates through all alarms and checks if the current time matches the alarm time. 

    - If an alarm needs to be triggered, it sets the isRinging flag to true and activates the buzzer.

        ```cpp
        void isAlarm(Alarm *alarms[]) {
            getTime(&timeinfo); 
            for (int i = 0; i < 3; i++) {
                if (alarms[i]->isSet && timeinfo.tm_hour == alarms[i]->time->hours && timeinfo.tm_min == alarms[i]->time->minutes) {
                    alarms[i]->isRinging = true;
                    ringingAlarm = true;
                    alarms[i]->isSet = false;
                    // Activate Buzzer here 
                    ledcWriteTone(0, 880); 
                } 
                // ...
            }
        }```

- Alarm Disabling:

    - The disableAlarm function sets the isSet and isRinging flags of an alarm to false, effectively disabling it. 
    - It also saves the updated state to non-volatile memory.
        ```cpp
        void disableAlarm(Alarm *alarm, Preferences *preferences) {
            alarm->isSet = false; 
            alarm->isRinging = false; 
            saveAlarm(alarm, preferences); // Save updated state
        }```

### Time Management

This component manages time synchronization and allows for user-defined time zone adjustments.

- Time Synchronization:

    - The setTime function synchronizes the system time with an NTP server. 
    - It uses configTime with the offset obtained from the offset struct and the daylightOffset_sec.
        ```cpp
        void setTime(Time offset, int daylightOffset_sec, const char *ntpServer) {
            int offsetInSeconds = offset.hours * 3600 + offset.minutes * 60; // Calculate total offset in seconds
            messageDisplay("Time", "Configuring..", timeConfig); 
            configTime(offsetInSeconds, daylightOffset_sec, ntpServer);
            // ...
        }
        ```
- Time Zone Offset:

    - Users can configure a custom time zone offset from UTC, which is used in the configTime function.

    - The offset struct holds the user-defined time zone offset, which is used in the configTime function. 
    - The offset is loaded from and saved to non-volatile memory using Preferences.
        ```cpp
        Time offset = {3, "Offset", 5, 30}; // Example initial offset

        void loadOffsets(Time *offset, Preferences *preferences) {
            preferences->begin(offset->time_name, false);
            offset->hours = preferences->getUInt("hour", offset->hours);
            offset->minutes = preferences->getUInt("minute", offset->minutes);
            preferences->end();
        }

        void saveOffset(Time *offset, Preferences *preferences) {
            // ... Similar to loadOffsets, but uses putUInt to save
        }
        ```

### User Interface

This layer manages user interactions with the system through the OLED display and buttons.

- Menu Navigation:
    - The handleMainMenu function manages the main menu displayed on the OLED. 
    - It uses the selectedFrame struct to track which menu option is currently selected. 
    - The goForwardButton and goBackwardButton are used to scroll through menu options.
        ```cpp
        bool handleMainMenu(Alarm *alarms[], Time *offset, SelectedFrame *selectedFrame, Menu *menu, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton) {
            pooling(goForwardButton, goBackwardButton, cancelButton); 
            if (goForwardButton->pressed) {
                if (selectedFrame->frameStartY < 50) {
                    selectedFrame->frameStartY += 10;
                    selectedOption = (MenuOptions)(selectedOption + 1); 
                } else {
                    selectedFrame->frameStartY = 10;
                    selectedOption = Alarm_01; 
                }
                // ...
                displayMenu(selectedFrame, menu);
            }
            // ... Similar logic for goBackwardButton and other buttons 
        }
        ```

- Alarm Configuration:

    - Functions like adjustAlarmHours and adjustAlarmMinutes are used to handle button presses and increment or decrement the alarm hours and minutes:

        ```cpp
        if (goForwardButton->pressed) {
            if (alarm->time->hours < 23) {
                alarm->time->hours++; 
            } else {
                alarm->time->hours = 0;
            }
            goForwardButton->pressed = false;
            goForwardButton->numberKeyPresses = 0;
            displayAlarm(alarm->time->hours, "Hours"); 
        }
        ```


- Time Zone Configuration:

    - The user can set the time zone offset using the menu navigation. 
    - Functions like adjustTimeZoneHours and adjustTimeZoneMinutes handle the incrementing and decrementing of the time zone hours and minutes based on button presses.
        ```cpp
        void adjustTimeZoneHours(Time *time, Button *menuButton, Button *goForwardButton, Button *goBackwardButton, Button *cancelButton) {
            pooling(goForwardButton, goBackwardButton, cancelButton);
            if (goForwardButton->pressed) {
                if (time->hours >= -13 && time->hours < 15) {
                    time->hours++;
                } else if (time->hours >= 15) {
                    time->hours = -13; 
                } else if (time->hours < -13) {
                    time->hours = 15; 
                }
                // ...
                displayTimeZone(time->hours, "Hours"); 
            }
            // ... Similar logic for decrementing and handling other buttons
        }
        ```

- Warning Display:

    - Utilizes the OLED display to present warning messages if temperature or humidity levels exceed pre-defined thresholds, ensuring that users are aware of potential issues.

    - The handleWarning function checks for temperature and humidity exceeding predefined limits. If a warning condition is detected, it updates the OLED display with relevant information.
        ```cpp
        bool handleWarning(DHTData dhtData) {
            bool warn = false;
            if (dhtData.temperature >= TEMPURATION_UPPER_LIMIT || dhtData.temperature <= TEMPURATION_LOWER_LIMIT) {
                display.setCursor(0, 48);
                display.println("Temperature: " + String(dhtData.temperature) + "C"); // Display warning message
                warn = true;
            }
            if (dhtData.humidity >= HUMIDITY_UPPER_LIMIT || dhtData.humidity <= HUMIDITY_LOWER_LIMIT) {
                display.setCursor(0, 56);
                display.println("Humidity: " + String(dhtData.humidity) + "%"); // Display warning message
                warn = true;
            }
            // ...
            return warn;
        }
        ```



- Status Display:

    - Displays essential information, including the current time, date, and environmental conditions (temperature, humidity) on the OLED screen:

        ```cpp
        display.setCursor(0, 48);
        display.drawRect(12, 14, 104, 28, SSD1306_WHITE);
        display.setCursor(34, 0);
        display.println(&timeinfo, "%d-%m-%Y");
        display.setCursor(16, 20);
        display.setTextSize(2);
        display.println(&timeinfo, "%H:%M:%S");
        ```

### Communication Management

This component manages communication with the external world using the MQTT protocol.

- MQTT Client:

    - Establishes a secure connection with the MQTT broker using the WiFiClientSecure and PubSubClient libraries:

        ```cpp
        WiFiClientSecure espClient;
        PubSubClient client(espClient);
        espClient.setCACert(root_ca);
        client.setServer(MQTT_SERVER, MQTT_PORT);
        ```

- Publish Sensor Data:

    - Publishes sensor readings (LDR values, temperature, humidity) to specified topics on the MQTT broker using client.publish:

        ```cpp
        char buffer[256];
        serializeJson(doc, buffer); // Serialize sensor data into JSON format
        client.publish(LDR_TOPIC, buffer);
        ```


- Subscribe to Servo Control:

    - Subscribes to a designated topic to receive control commands for the servo motor from the MQTT broker using client.subscribe:

        ```cpp
        client.subscribe(SERVO_TOPIC);
        ```

    - The callback function (callback) processes received commands and controls the servo motor accordingly:

        ```cpp
        void callback(char *topic, byte *payload, unsigned int length) {
            String incommingMessage = ""; 
            // ... process received message ... 
            if (topic_Str == SERVO_TOPIC) {
                handleServoMotor(incommingMessage, &servo); 
            }
        }
        ```

<!-- 4. Data Flow Diagram
[Include a Data Flow Diagram illustrating the data exchange between different components of the system, including the sensors, actuators, user interface, MQTT broker, and Node-RED dashboard.]
5. Software Architecture Diagram
[Include a Software Architecture Diagram visualizing the interaction between different software components and their functionalities.]
6. Deployment Diagram
[Include a Deployment Diagram illustrating the physical deployment of the software components on the ESP32 hardware.] -->
## Non-Functional Requirements
### Power Management
- The system utilizes light sleep mode for power conservation when not actively processing tasks.
- On Change Detection is implemented for sensors, triggering readings only when significant changes occur, minimizing unnecessary power consumption.
### Data Persistence
- Alarm configurations and user settings are saved to non-volatile memory using the Preferences library, ensuring data retention even after power loss.
### Security
- Secure communication with the MQTT broker is established using TLS/SSL encryption through the WiFiClientSecure library, protecting sensitive data during transmission.
### Scalability
- The modular design facilitates the integration of additional sensors and actuators. 
- The hardware abstraction layer provides flexibility in adapting to different hardware configurations.
## Conclusion
The MediBox software architecture effectively integrates various hardware and software components to deliver a comprehensive smart pharmaceutical storage system. The modular design promotes maintainability, scalability, and flexibility for future enhancements. The system's robust functionality, coupled with its advanced features, provides users with a reliable and user-friendly solution for managing medication storage and dispensing.