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

<img src="Readme Images/Screen Shot 2022-09-01 at 1.21.11 PM.png" width="200">

## Receiving Infrared signal:
We receive a PWM signal from the remote through a phototransistor. We must convert this
signal into a differential speed which can then be used to control the direction of the car. The
input that the microcontroller received was the PWM signal from the infrared LED. From this
signal a differential speed for each wheel is calculated

## Setting wheel speed based on infrared signal:
Every 100 ms we will take the current differential speed from the input
PWM and set a target speed for each wheel based on that speed. Then we
will implement an integral control scheme for accuracy of speed. We will
finally take the PWM output of the integral control and send it to the wheels
to modify the speed.

<img src="Readme Images/Screen Shot 2022-09-01 at 1.21.18 PM.png" width="200">

These 3 main parts work together to convert a user input from the
potentiometer to a physical change in the speed of the car. Data is passed
through different mediums across each part to eventually represent this
physical change. We start with a sensory input which is transmitted through
the air in the form of infrared light. This is then transferred through internal
processes of the microcontroller to resemble a PWM signal, which is
converted through the electrical and mechanical processes of the DC motor
to represent a physical wheel speed that represents the specific user input.

# Low-Level Design:

<img src="Readme Images/Screen Shot 2022-09-01 at 1.21.31 PM.png" width="800">

## Sending infrared signal:
A pulse-width-modulated infrared signal was sent from the remote microcontroller(right on circuit
diagram) based on a rotary potentiometer representing direction. The potentiometer voltage was
sent to the ADC and converted to a 14 bit number. This number was converted to a compare
value from 50 to 799 out of 800 timer counts representing the PWM duty cycle of the infrared
signal based on the equation:

`uint16_t 𝑥 = (𝐴𝐷𝐶 𝑟𝑒𝑠𝑢𝑙𝑡)*749/16384 + 50`
