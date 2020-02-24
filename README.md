# Turbine

Node.js express application to control the turbine blade speed of a 3D printed wind turbine, controlled by a rasperry pi zero contained in the base.   The turbine itself can be printed and constructed from the following [instructions](https://github.com/AndyRWatson/turbine3D).

The turbine itself is controlled by the endpoint

```
localhost:<port>/setTurbine?speed=** <speed> **
```
where  **speed** is a numeric value in the range *1* (fast) and *100* (slow/off), or a value off **off** or **on**


## Setup Instructions

Git clone this repo into your local file system

<details>
  <summary>Raspberry Pi</summary>
  
  ## Raspberry PI
1. Create a raspbian disk image for use by the raspberry pi zero
* download from https://www.raspberrypi.org/downloads/
* create sd using
2. enable ssh support  (raspi-config)
3. cd into Turbine directory
4. run *./start.sh*
5. start the turbine express application by running *npm start*

</details>

<details>
  <summary>NodeMCU (Arduino)</summary>
  
  ## nodeMCU (Arduino)
  1. Download Arduino IDE
     * https://www.arduino.cc/en/main/software
  2. Open Arduino IDE and open project
    * [./Turbine/Arduino/turbine.ino](https://github.com/AndyRWatson/Turbine/Arduino/Turbine.ino).
  3. Configure board and port
  4. Install prereq packages
  5. Upload to board
</details>


# Turbine
