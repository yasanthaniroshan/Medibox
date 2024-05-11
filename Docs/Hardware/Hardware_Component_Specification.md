# Hardware Components Specification

## Components

### ESP32 Development Board

![ESP32](/Design/Esp32.jpg)

**Description:**
The ESP32 development board serves as the central processing unit of the MediBox system, responsible for executing the embedded software, controlling peripheral devices, and managing communication with the MQTT broker.

**Features:**
- Integrated Wi-Fi and Bluetooth connectivity
- Dual-core processor for efficient multitasking
- Multiple GPIO pins for interfacing with sensors and actuators
- Low power consumption
- Support for various communication protocols

**Technical Specifications:**
- Chipset: ESP32
- Dimensions: 58.4mm x 25.4mm
- Weight: 5g
- Power Requirements: 5V DC
- Clock Speed: Up to 240MHz
- Interfaces: UART, SPI, I2C, GPIO
- Port Mapping: Refer to the ESP32 pinout diagram for specific port mappings based on the development board model.
- Compatibility: Compatible with PlatformIO and other embedded software development platforms.
- Environmental Requirements: Operating Temperature: 0°C to 55°C, Operating Humidity: 5% to 95% RH (non-condensing)
- Installation Instructions:
    - Connect the ESP32 development board to a computer via USB.
    - Install the necessary drivers and development tools.
    - Upload the MediBox embedded software to the ESP32 board.
- Maintenance and Support: No specific maintenance is required.
- Troubleshooting: Verify proper power supply and connections. Consult the ESP32 documentation for support and troubleshooting.
- References: [Espressif Systems](https://www.espressif.com/)

### OLED Display (SSD1306)

![OLED](/Design/ssd1306.jpeg)

**Description:**
The OLED display provides a visual interface for the MediBox system, displaying the current time, date, sensor readings, alarm status, and user menus.

**Features:**
- High contrast and wide viewing angle
- Low power consumption
- Compact size
- I2C communication interface

**Technical Specifications:**
- Display Size: 128x64 pixels
- Dimensions: 27mm x 27mm
- Weight: 2g
- Power Requirements: 3.3V DC
- Interfaces: I2C
- Port Mapping:
    - SDA: GPIO 21
    - SCL: GPIO 22
- Compatibility: Compatible with Adafruit SSD1306 library and other compatible libraries.
- Environmental Requirements: Operating Temperature: -40°C to 85°C
- Installation Instructions:
    - Connect the OLED display to the ESP32 development board using I2C communication lines.
    - Install the Adafruit SSD1306 library.
    - Configure the library for the OLED display's I2C address.
- Maintenance and Support: No specific maintenance is required.
- Troubleshooting: Verify proper power supply and connections.
- References: [Adafruit](https://www.adafruit.com/)

### Buzzer

![Buzzer](/Design/Buzzer.jpeg)

**Description:**
The buzzer serves as an audible alarm indicator, notifying users when a medication alarm is triggered.

**Features:**
- Piezoelectric buzzer
- Produces a loud and clear alarm sound
- Low power consumption

**Technical Specifications:**
- Dimensions: 12mm x 12mm
- Weight: 1g
- Power Requirements: 5V DC
- Interfaces: GPIO
- Port Mapping: GPIO 25
- Compatibility: Compatible with standard GPIO interfaces.
- Environmental Requirements: Operating Temperature: -20°C to 70°C
- Installation Instructions:
    - Connect the buzzer's positive (+) pin to GPIO 25 and the negative (-) pin to ground (GND).
- Maintenance and Support: No specific maintenance is required.
- Troubleshooting: Verify proper power supply and connections. Test the buzzer's functionality using a multimeter.
- References: [Arduino](https://www.arduino.cc/)

### Push Buttons

![Push Buttons](/Design/push_button.jpeg)

**Description:**
The push buttons provide user input for navigating menus, configuring settings, and interacting with the MediBox system.

**Features:**
- Tactile feedback
- Normally open configuration
- GPIO interface

**Technical Specifications:**
- Dimensions: 6mm x 6mm
- Weight: 1g
- Power Requirements: N/A
- Interfaces: GPIO
- Port Mapping:
    - Menu Button: GPIO 23
    - Go Forward Button: GPIO 5
    - Go Backward Button: GPIO 18
    - Cancel Button: GPIO 19
- Compatibility: Compatible with standard GPIO interfaces.
- Environmental Requirements: Operating Temperature: -20°C to 70°C
- Installation Instructions:
    - Connect each button's pin to its designated GPIO pin on the ESP32 development board.
    - Connect the other pin of each button to ground (GND).
- Maintenance and Support: No specific maintenance is required.
- Troubleshooting: Verify proper connections and button functionality using a multimeter.
- References: [Arduino](https://www.arduino.cc/)

### Light Dependent Resistors (LDRs)

![LDRs](/Design/ldr.jpeg)

**Description:**
The LDRs measure ambient light intensity, providing data for adjusting the shaded sliding window position based on user-defined parameters for different medicines.

**Features:**
- Variable resistance based on light intensity
- Analog output
- Low cost

**Technical Specifications:**
- Resistance Range: 1kΩ to 100kΩ (depending on light intensity)
- Dimensions: 5mm x 5mm
- Weight: 1g
- Power Requirements: N/A
- Interfaces: Analog Input
- Port Mapping:
    - Left LDR: GPIO 35
    - Right LDR: GPIO 34
- Compatibility: Compatible with standard analog input interfaces.
- Environmental Requirements: Operating Temperature: -20°C to 70°C
- Installation Instructions:
    - Connect one leg of each LDR to its designated GPIO pin on the ESP32 development board.
    - Connect the other leg of each LDR to a 10kΩ resistor.
    - Connect the other end of the resistor to 3.3V.
- Maintenance and Support: No specific maintenance is required.
- Troubleshooting: Verify proper connections and LDR functionality using a multimeter. Calibrate the LDR readings if necessary.
- References: [Arduino](https://www.arduino.cc/)

### Servo Motor

![Servo Motor](/Design/servoMotor.jpeg =500x)

**Description:**
The servo motor controls the position of the shaded sliding window, adjusting the amount of light exposure based on the measured light intensity and user-defined parameters.

**Features:**
- Precise angle control
- PWM control interface
- High torque

**Technical Specifications:**
- Operating Voltage: 5V DC
- Operating Current: 500mA
- Rotation Angle: 180 degrees
- Dimensions: 40mm x 20mm x 38mm
- Weight: 40g
- Interfaces: PWM
- Port Mapping: GPIO 33
- Compatibility: Compatible with standard servo motor libraries.
- Environmental Requirements: Operating Temperature: 0°C to 55°C
- Installation Instructions:
    - Connect the servo motor's control wire to GPIO 33 on the ESP32 development board.
    - Connect the servo motor's power wire to 5V and the ground wire to GND.
- Maintenance and Support: No specific maintenance is required.
- Troubleshooting: Verify proper power supply and connections. Check the servo motor's functionality using a servo motor tester.
- References: [Arduino](https://www.arduino.cc/)

## Port Map

| Component                | GPIO Pin |
|--------------------------|----------|
| OLED Display (SSD1306)   | SDA - GPIO21 |
|                          | SCL - GPIO22 |
| Buzzer                   | GPIO25 |
| Push Buttons             | Menu Button - GPIO23 |
|                          | Go Forward Button - GPIO5 |
|                          | Go Backward Button - GPIO18 |
|                          | Cancel Button - GPIO19 |
| Light Dependent Resistors| Left LDR - GPIO35 |
|                          | Right LDR - GPIO34 |
| Servo Motor              | GPIO33 |

------------------ 

![here](/Design/ESP32-pinout-diagram.jpg)