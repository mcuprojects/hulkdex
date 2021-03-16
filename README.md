# Hackdex
Is a manually operated Arduino-based controller for Haldex gen1 AWD system used in Audi TT mk1 (1999 - 2006) VW Golf mk4 (4-motion) and many other vehicles.
The controller is intended to be a replacement for OEM Haldex unit controller with a simple ON/OFF control.
In order to use this controller the mechanical part (pump/clutch/oil/drivetrain etc.) of your Haldex system needs to be in working condition, while the electric part (controller) may be faulty.
Installing this controller enables you to manually select between FWD/AWD using a button inside the car. This can be a solution if your original Haldex controller does not work or does not work properly or you simply want to have a manual control over the AWD.

Hackdex is cheap to make and gives the same benefits as the so-called 'sport insert' (https://www.ttforum.co.uk/forum/viewtopic.php?t=1398266) but without drawbacks of having AWD enabled all the time, which may be dangerous at high speeds or may cause unnecesary wear on the Haldex system. In addition, this controller uses built-in Haldex temperature sensor to prevent overheating.

# Background / info

# Hardware
This controller uses Arduino-Nano and L298N board to control the original Haldex stepper motor to open/close the hydraulic valve.
Additionaly DC-DC step-down converter is needed to make 5V power for Arduino. Plus some resistors/transistors and a 5V controlled relay.
##№ Schematics / wiring
![Picture of connection points on the OEM controller]
![Schematics]
![Photo of the controller assembled]

# Software
Nothing special here, just use Arduino IDE to build and flash the project onto your device.

# Installation guide
### 1. Remove the Haldex system from your car.
While you're at it, it may be a good time to change the oil/filter and check if the pump works.
### 2. Unscrew the electronic control unit.
### 3. Disassemble the control unit.
### 4. Remove the stepper motor and examine the oil condition underneath.
If the oil is dirty it can jam the motor sooner or later. It is recommended to replace the motor. A good replacement is NEMA 17 34mm height motor.
### 5. [optional but advisable] Remove the valve plug and inspect the valve condition.
Replace O-rings if needed
### 6. Connect 4.7 kOhm resistor in place of original temperature sensor.
### 7. Wiring out from OEM controller enclosure.
### 8. Put the controller enclosure back together.
### 9. Install hackdex box somewhere on the Haldex unit and connect all connectors.
### 10. Wire a USB cable (optional) and a cable to in-car button.
### 11. Install everything back on the car.
