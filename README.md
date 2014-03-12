SC_Rocketry14
=============

Rocket Avionics Code

This repository contains all of the code necessary for testing these sensorson their own, as well as together as the overall control system.

Hardware:
-Arduino Microcontroller (code is currently written for Mega2560, NOT Due)
-Sparkfun 9 Degrees of Freedom Sensor stick (ADXL345, HMC5883L, ITG-3200)
-OpenLog Serial to microSD logger
-Adafruit BMP180 pressure sensor breakout board
-Adafruit Ultimate GPS (built on MTK3339 chipset)
-Xbee Pro Series 2





To Do:
-Take an average of values for pressure/vertical acceleration to ensure there are no unwanted changes in state of the control system (for example from Drogue --> Main before 1800 ft.)
-Incorporate 9-DOF data fetching function (get data from 9-DOF, store in "dofData[]" global array)
-Write a function to calculate the vertical acceleration of the rocket (vertAccel) from the accelerometer data
-Test different serial writing speeds for OpenLog and the Xbees
