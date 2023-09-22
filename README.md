# arduboy_raycast
A raycasting library for arduboy

<img src="https://qcs.shsbs.xyz/api/file/raw/jmsdk" width="256">

Please see the examples directory. This project is currently under development

## How to install

This library depends on **FixedPoints.h** and **Arduboy2.h**. You must have those installed first.

Until the library is more stable, I won't be adding it to the Arduino library manager. As such, you have
a couple options:

### Direct include 
Download the src directory into your project, perhaps rename the folder "rclib", then do `#include "rclib/ArduboyRaycast.h"`

### Arduino library
If on Windows, you can download the entire repo (inluding the root files such as library.properties) and place the contents
within `Documents\Arduino\libraries\ArduboyRaycast`. This means the `src` folder and `library.properties` should be 
directly within `Documents\Arduino\libraries\ArduboyRaycast` and no deeper. Now you can `#include <ArduboyRaycast.h>` 
in any arduboy project.

