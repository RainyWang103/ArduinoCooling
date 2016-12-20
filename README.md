# ArduinoCooling
The Project is a cooling system control logic implemented on Arduino.cc with hardware supports


## Introduction to Arduino

Official website: https://www.arduino.cc/

Arduino.cc is an Open-source hardware shares much of the principles and approach of free and open-source software. To facilitate flexible hardware implementation, Arcuino releases all of the original design files (Eagle CAD) for the Arduino hardware. These files are licensed under a Creative Commons Attribution Share-Alike license, which allows for both personal and commercial derivative works, as long as they credit Arduino and release their designs under the same license.

The Arduino software is also open-source. The source code for the Java environment is released under the GPL and the C/C++ microcontroller libraries are under the LGPL.

For desired libraries, find at: https://www.arduino.cc/en/Reference/Libraries
Documentation: https://www.arduino.cc/en/Reference/HomePage



## Introduction to project

The Project is a cooling system control logic implemented on Arduino.cc with hardware supports. It was built with temperature sensors, IRremote receiver and AC/DC circuit modules. The codes contained in this project only specifiy the control logic and function realization of the cooling system modes. The detailed information can be seen in the comment of relevant code documents.

To realize the whole system, you may also need a electrical module which converts 220V power source to the level you want. This project also defines how to read runtime data (ac and dc current in this case) from the electrical part. It is an important feedback loop to keep track of everything and interrupt the system when things go crazy.
