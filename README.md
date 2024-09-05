# motion_capture_stream

> [!NOTE]
> 1. tested on Windows10
> 
> 2. supported system: [OptiTrack](https://optitrack.com/)'s [Motive](https://optitrack.com/software/motive/)

## Prerequisite
### NatNet SDK
- NatNet SDK version 4.1.1 is already shipped with this repository. This is the only tested version.
- If you want other versions, download whatever from [here](https://optitrack.com/support/downloads/developer-tools.html). (not tested)

### MSVC (for Windows)
- Install MSVC (via [Visual Studio](https://visualstudio.microsoft.com/ko/vs/community/))
- NatNet SDK's Windows libraries are only compatible with MSVC (incompatible with minGW).

## Build
### Windows
```
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release
```
- Then, the **executible** is located at `build/Release`, i.e., `build/Release/OptitrackStreaming.exe`.
  - The executible is shipped along with dynamic library `NatNetLib.dll`
- If you have downloaded other version of MSVC:
  - Check lists of supported generators using `cmake --help` and Generators field therein.
  - Typically: `Visual Studio 17 2022` or `Visual Studio 16 2019` with options `-A` (architecture) `x64` (64bit).

### Ubuntu Linux (WIP)

## Usage
WIP