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

## FX library
By default, this library loads sprites and tiles from program memory. This is significantly
faster and somewhat more usable, but the sprites and tiles take up a lot of program memory.

You can instead load sprites and tiles from the FX chip by including `ArduboyRaycastFX.h`
*instead of* `ArduboyRaycast.h` and creating the appropriate fxdata. You shouldn't need to
change any code other than to pass pointers into the FX for your tilesheet and spritesheet 
rather than progmem pointers. There are several benefits:
- Tiles and sprites are now 32x32
- You specify custom mipmap levels for 16x16, 8x8, and 4x4 versions of tiles and sprites.
  This can improve texture quality, but is mostly required for performance
- No resources stored in program memory anymore; you can fill up the entire 256 tiles 
  and 256 sprites available to you with no impact on codesize

There are MAJOR caveats to using the FX:
- Performance takes a steep penalty, greater than 30% reduction I believe.
- Performance is penalized even if you use 16x16 tiles, as the 32x32 tilesize is
  still *required*
- ALL mipmap levels are required, so you will need 32x32, 16x16, 8x8, and 4x4 versions
  of your tiles and sprites.
- The format of images on the FX chip is different in order to maximize efficiency.
  As such, generating the fxdata is nontrivial. [Ardugotools](https://github.com/randomouscrap98/ardugotools)
  is, as of writing, the only way to automatically generate data in this format. 

If you'd like an example of using the FX library, as well as an example of utilizing 
Ardugotools to generate the FX data, please see [Example 7_fx](https://github.com/randomouscrap98/arduboy_raycast/tree/main/examples/7_fx)
