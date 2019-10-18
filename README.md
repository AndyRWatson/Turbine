# Turbine

Node.js express application to control the turbine blade speed of a 3D printed wind turbine, controlled by a rasperry pi zero contained in the base.   The turbine itself can be printed and constructed from the following [instructions](https://github.com/AndyRWatson/turbine3D).

The turbine itself is controlled by the endpoint

```
localhost:3000/setTurbine?speed=** <speed> **
```
where  **speed** is a numeric value in the range *1* (fast) and *100* (slow/off), or a value off **off** or **on**



## Setup Instructions

1. Create a raspbian disk image for use by the raspberry pi zero
2. enable ssh support  (raspi-config)
3. git clone this repository into the user directory
4. cd into Turbine directory
5. run *./start.sh*
6. start the turbine by running *npm start*

# Turbine
