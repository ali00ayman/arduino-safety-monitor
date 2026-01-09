# Arduino Safety Monitor

## Overview
This project is a safety and environment monitoring system built using an Arduino Uno R3. It detects gas leakage, monitors temperature and humidity, and measures the distance of nearby objects. All sensor data and warnings are shown on a 0.96-inch OLED display, with visual and sound alerts.

## How the System Works
- The MQ-135 gas sensor monitors air quality.
- The DHT11 sensor measures temperature and humidity.
- The ultrasonic sensor measures the distance of objects from the device.
- A 0.96-inch OLED screen displays live data and warning messages.
- RGB LEDs and a buzzer provide visual and sound alerts.

### Warning Logic
- If an object is closer than **30 cm**, the buzzer sounds and the **yellow LED** flashes.
- If air quality exceeds **90**, the buzzer sounds and the **yellow LED** flashes.
- If **both conditions** are detected at the same time, the **red LED** flashes and the buzzer sounds continuously.

### Display Warnings
- **[WARNING] AIR** → Gas level warning  
- **[WARNING] OBJECT** → Object too close  
- **[!! EMERGENCY !!]** → Gas and object detected together  

A hazard icon is shown in the top corner of the screen:
- One warning: a frowning stick figure  
- Emergency: a waving stick figure with an exclamation mark  

## Components Used
- Arduino Uno R3  
- MQ-135 gas sensor  
- DHT11 3-pin temperature and humidity sensor  
- 0.96-inch 4-pin blue OLED display  
- Ultrasonic sensor  
- Red, Green, and Yellow RGB LEDs  
- 3 × 220Ω resistors  
- 5V magnetic buzzer  
- Male-to-male wires  
- Female-to-male wires
- Code: used chatgpt to generate it 

## Circuit Diagram
![Circuit Diagram](images/Digital%20Prototype.png)
