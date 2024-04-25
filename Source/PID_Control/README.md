# Arduino PID Motor Controller

## Introduction

This project encompasses an Arduino sketch designed to control a DC motor's speed with precision using a PID (Proportional, Integral, Derivative) controller. The implementation utilises a rotary encoder for real-time feedback on the motor's position and velocity. 

## Diagram

![PID Control System Diagram](/Source/PID_Control/Pictures/PID.png)


## User Installation Instructions

To use this motor controller, you will need the Arduino IDE and the appropriate hardware setup:

1. Install the [Arduino IDE](https://www.arduino.cc/en/software).
2. Assemble your hardware according to the circuit diagram provided above.
3. Connect the Arduino to your computer via a USB cable.

## How to Run the Code

After setting up the hardware:

1. Open the Arduino IDE.
2. Go to `File` > `Open...` and select the `.ino` file from this repository.
3. Select the correct board and port via `Tools` > `Board` and `Tools` > `Port`.
4. Click on the `Upload` button to upload the code to your Arduino.
5. Open the `Serial Monitor` to see the live RPM readings.

## More Technical Details

The PID control system is governed by three parameters: the proportional gain (`kp`), the integral gain (`ki`), and the derivative gain (`kd`). These parameters influence the controller's response to the error between the desired and the actual speed of the motor, with each parameter playing a role in minimising this error over time.

## Known Issues/Future Improvements

- Current control parameters may need to be adjusted for different motors and loads.
- Future iterations could include a user interface for real-time parameter tuning.
- Integration of a comprehensive error handling mechanism to improve robustness.


