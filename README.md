# RUNS_CLIENT

GUI companion app for the RUNS robot platform.

[Firmware repository]([url](https://github.com/Jah-On/RUNS_FIRMWARE)).

![image](https://github.com/Jah-On/RUNS_CLIENT/assets/58399643/dbf004c2-17c2-4c75-93bb-4219f0821241)

<!-- TODO: Add screenshot and maybe demo video -->

## Build steps

- Clone the repo: `git clone https://github.com/Jah-On/RUNS_CLIENT`
- Change directory: `cd RUNS_CLIENT`
- Fetch submodules: `git submodule update --init --recursive`
- Run CMake: `cmake .`
- Build: `make`

## Usage notes

To run the app, do `./RUNS_CLIENT` in the project's root directory. You must connect to the robot via your system's Bluetooth settings.

### Inputs

#### Keys

- `Esc`   - Exit application
- `Space` - Halt/Stop
- `W`     - Increase Speed
- `S`     - Decrease Speed
- `A`     - Rotate   Right
- `D`     - Rotate   Left

#### UI Input

The speed slider in the bottom center can change speed with mouse input. The left and right triangles are not buttons and thus cannot be controlled by the mouse.

#### Other important notes

The speed will reset to zero if any of the bumpers are triggered because the firmware stops the robot in this event.
