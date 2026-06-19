This project was programmed for and on Mr. Kang's hardware. It was his idea and his goals that this code seeks to fulfill.

# Goal
A device that can detect the presence of a basketball. 
When that basketball is removed, a timer starts counting down to zero. 
Once the timer hits zero, a buzzer is played. 
The timer can be changed via the two buttons, or by the reset button on the board itself.

# List of Parts
* Arduino Nano board
* Seeed Studio Grove Shield for Arduino Nano, v1.3 07/26/2023
* Seeed Studio Buzzer v1.3
* 4-digit display (possible model number 3642BS-1 with separate 420)
* Grove Dual Button V1.0
* Ultrasonic Distance Sensor V2.0

# Configurable In-Code
* All the pins can be changed if the device layout changes.
* The cooldown for button presses (default 100 milliseconds)
* The ball detection threshold in centimeters (default 20 centimeters)
* The buzzer duration (default 1000 milliseconds/1 second)
