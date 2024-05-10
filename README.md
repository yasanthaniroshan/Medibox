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
    - [Arduino IDE](https://www.arduino.cc/en/software)
    - [Python](https://www.python.org/)


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
  
3. Run the Project

    - Open the project in VSCode and build the project using the platform.io plugin.
    - Upload the firmware to the ESP32 development board.
    - Run the Node-RED server using the command `node-red` in the terminal.
    - Open the Node-RED dashboard in the browser using the URL `http://localhost:1880/ui`.


## Simulation

- First you have to setup Workwi simulator for the project. The setup guide can be found [here](https://docs.wokwi.com/vscode/getting-started)
- The project can be simulated using the wokwi simulator. The simulation can be found [here](Simulation/diagram.json)

## Documentation

- Requirment Documents 
    - [Document 01](Docs/In21-EN2853-Programming_Assignment_1.pdf)
    - [Document 02](Docs/In21-EN2853-Programming_Assignment_2.pdf)

## License

[MIT](https://choosealicense.com/licenses/mit/)

