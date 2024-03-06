# MediBox

A embebedd project which mainly focused for the Medibox where tempurature and humidity control is essential.

## Optimizations

- Interrupt driven architecture
    - Instead of using pooling method,in here interrupt routes are used for display update,set alarms
- Power Management
    - Once data is updated in the display,ESP32 device will be going to sleep mode which pauses the CPU operation and enter to **light_sleep** mode
        ### Power Consumption Comparison
        - **Without Power Management (Using pooling methods)**
            $$ Total\ Power\ Consumption = V \times I  = 3.3V \times 100 mA = 330\,mW$$
        - **With Power Management (Using interrupts)**
            $$ Total\ Power\ Consumption = V \times I  = 0.9\times(3.3V \times 0.8 mA) + 0.1 \times (3.3V \times 100 mA) = 35.376\,mW$$
        - **Average Saving**
            $$ Power\ Saving  = \dfrac{294.624}{330}\times 100 \% = \mathbf{89.2}\%$$
    > [!NOTE]
    > These values can be changed according to the device and Average using time
            
            
## Screenshots

![Power Consumption Table](./screenshots/power_consumption.jpg)

