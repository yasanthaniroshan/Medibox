# MediBox

![MediBox Logo](/Design/MediBoxLogo.png)

## Introduction

The MediBox Embedded Software Project aims to develop a smart pharmaceutical storage system capable of managing medicines effectively. This documentation outlines the design, implementation, and functionalities of the embedded software that controls the MediBox hardware components.

## System Overview

The MediBox system consists of hardware components including an **OLED screen**, **buzzer**, **push buttons**, **Light Dependent Resistors (LDRs)**, and a **servo motor**. The software components include embedded software for hardware control, NTP client for time synchronization, alarm management system, temperature/humidity monitoring, light intensity monitoring, and a user interface for configuration and interaction.

## Functionalities

The software provides the following functionalities:
- Set time zone and synchronize time with NTP server.
- Set and manage alarms, including disabling all alarms.
- Display current time on the OLED screen.
- Ring alarms with proper indication when triggered, stop alarms using push buttons.
- Monitor temperature and humidity levels, provide warnings if exceeded healthy limits.
- Monitor light intensity using LDRs, display real-time and historical data on Node-RED dashboard.
- Adjust shaded sliding window position using servo motor based on light intensity and user-defined parameters for different medicines.

## Advanced Functionalities

The software provides the following advanced functionalities:
- Persist alarms and user settings in non-volatile memory.
- User Friendly OLED screen interface for configuration and interaction.
- Advaced power management system to reduce power consumption.
- On Change Detection for sensors to reduce power consumption.
- Continous monitoring of temperature, humidity and light intensity.

## Software Architecture

The MediBox software utilizes a modular architecture, dividing the system into distinct components to ensure efficient management of functionalities
1. Hardware Abstraction Layer
    - Provides a simplified interface for interacting with hardware components like the OLED display, buzzer, buttons, LDR sensors, and servo motor.
    - Abstracts hardware-specific details, allowing for easier software development and potential adaptation to different hardware configurations.
2. Sensor Management
    - Handles data acquisition from the DHT sensor (temperature and humidity) and LDRs (light intensity).
    - Processes and stores the collected sensor data, enabling environmental monitoring and data visualization on the Node-RED dashboard.
3. Alarm Management
    - Enables users to set, disable, and manage medication alarms.
    - Activates the buzzer and displays a notification on the OLED screen when an alarm is triggered, reminding users to take their medication.
    - Stores alarm configurations in non-volatile memory, ensuring persistence across power cycles.
4. Time Management
    - Synchronizes the system time with an NTP server, ensuring accurate timekeeping.
    - Allows users to configure a custom time zone offset from UTC, tailoring the system to their location.
    - Saves the time zone offset in non-volatile memory for persistent operation.
5. User Interface
    - Provides a menu-driven interface on the OLED display, enabling users to navigate through different functionalities using the push buttons.
    - Allows for configuring alarms, setting time zones, and viewing system status information.
    - Displays warning messages when temperature or humidity levels exceed predefined thresholds.
6. Communication Management
    - Establishes a secure connection with an MQTT broker for remote data transmission and control.
    - Publishes sensor data (temperature, humidity, light intensity) to designated topics on the MQTT broker for visualization and analysis on the Node-RED dashboard.
    - Subscribes to a specific topic to receive control commands for the servo motor from the MQTT broker, enabling remote adjustments to the shaded sliding window.

This modular architecture ensures clear separation of concerns, enhances maintainability, and allows for future expansion of functionalities and integration of additional components.

> [!NOTE]
> Full Documentation about Softaware Architecture can be found [here](Docs/Software/Softaware_Architecture.md)

## Requirements

1. Hardware Components
    - [ESP32 Development Board](Design/Esp32.jpg)
    - [OLED Display](Design/ssd1306.jpeg)
    - [Buzzer](Design/Buzzer.jpeg)
    - [Push Buttons](Design/push_button.jpeg)
    - [Light Dependent Resistors (LDRs)](Design/ldr.jpeg)
    - [Servo Motor](Design/servoMotor.jpeg)

2. Software Components

    - [PlatformIO](https://platformio.org/)
    - [Node-RED](https://nodered.org/)
    - [MQTT Broker](https://mosquitto.org/)
    - [VSCode](https://code.visualstudio.com/)

## Installation

1. Hardware Installation

    - Connect the components as given in the Schemetic Diagram
    [Schemetic Diagram](Design/Medibox.png)

2. Software Installation

    - Clone this repository
        ```bash
        git clone https://github.com/yasanthaniroshan/Medibox.git
        ```
        
    - Setup [platform.io](https://platformio.org/) plugin for [VSCode](https://docs.platformio.org/en/stable/integration/ide/vscode.html#installation)

    - Install Node-RED as given in the [official documentation](https://nodered.org/docs/getting-started/)

    - Import the Node-RED flow from the [Node-RED Flow](Dashboard/dashboard.json)
  
3. Run the Project

    - Open the project in VSCode and Create Certificate.h to include your CA certificate as `const char *root_ca = R"EOF(...)EOF";`
    - Build the project using the platform.io plugin.
    - Upload the firmware to the ESP32 development board.
    - Run the Node-RED server using the command `node-red` in the terminal.
    - Open the Node-RED dashboard in the browser using the URL `http://localhost:1880/ui`.


## Simulation

- First you have to setup Workwi simulator for the project. The setup guide can be found [here](https://docs.wokwi.com/vscode/getting-started) and the simulation can be found [here](Simulation/diagram.json)

## Documentation

- Requirment Documents 
    - [Document 01](Docs/Requirements/In21-EN2853-Programming_Assignment_1.pdf)
    - [Document 02](Docs/Requirements/In21_EN2853_Programming_Assignment_2.pdf)

## License

[MIT](https://choosealicense.com/licenses/mit/)

