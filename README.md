# IR-RC-car
Project for an infrared remote controlled car using 2 TI MSP-EXP432P401R microcontrollers.

# Project Introduction and Description:
For this project we were tasked with designing and implementing a system embedded in a
microcontroller robotic car that would allow the car to successfully traverse a variable maze of
obstacles. Programming predefined paths were not allowed as a solution to this project and
would only only show completion of a fixed maze, and would not show the ability to adapt to a
change in structure of the maze from trial to trial.

# Proposed and Final Solutions:
In order to solve this maze, we developed a system of remote control to change the direction of
the car moving at a constant speed through an infrared signal, using 2 microcontrollers: one for
the car and one for the remote. The controller consisted of a rotary potentiometer, where the
direction in which the potentiometer was pointing represented the direction in which the car
would turn. When the infrared signal was out of range, the car would continue turning in the last
received direction from the remote. With this solution, we were able to easily steer the car
through the maze without any trouble.

# High-Level Design:
The main function of this system can be broken down into 3 main parts: sending infrared signal,
receiving infrared signal, and setting wheel speed based on that infrared signal.

### Sending infrared signal:
The infrared pulse-width-modulated signal is generated based on the ADC value of a
potentiometer. Every 100ms the ADC value corresponding to potentiometer voltage is read and
converted to a PWM signal which is sent to an infrared LED.
