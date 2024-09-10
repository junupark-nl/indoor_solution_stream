# Indoor Solution Stream

This repository maintains source codes with purposes of
1. streaming **motion capture** data
2. streaming **json data** conveyed over **UDP** (specific ip & port)

Here, the `streaming` includes
1. showing data to iostream
2. saving it on a .csv file

## Prerequisite
#### MSVC (for Windows)
- Install MSVC (via [Visual Studio](https://visualstudio.microsoft.com/ko/vs/community/))
- NatNet SDK's Windows libraries are only compatible with MSVC (incompatible with minGW).

#### NatNet SDK: Motion capture data
- NatNet SDK version 4.1.1 is already shipped with this repository. This is the only tested version.
- If you want other versions, download whatever from [here](https://optitrack.com/support/downloads/developer-tools.html). (not tested)

#### JSON C++: Json data (over UDP)
- already shipped with this repository.

## Motion capture
> [!NOTE]
> - Tested on Windows10
> - supported system: [OptiTrack](https://optitrack.com/)'s [Motive](https://optitrack.com/software/motive/)

### Build
#### Windows
```shell
cd motion_capture
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release
```
- Then, the **executible** will be located at `motion_capture/build/Release`, i.e., `OptitrackStreaming.exe`.
  - The executible is installed along with dynamic library `NatNetLib.dll`
- If you have downloaded other version of MSVC:
  - Check lists of supported generators using `cmake --help` and Generators field therein.
  - Typically: `Visual Studio 17 2022` or `Visual Studio 16 2019` with options `-A` (architecture) `x64` (64bit).

#### Ubuntu Linux
(WIP)

### Usage
- Build the project.
- Execute it as
  - Windows: `./motion_capture/build/Release/OptitrackStreaming.exe`
  - Linux: `./motion_capture/build/Release/OptitrackStreaming` (WIP)
- log files will be saved at `<project_root>/logs/motion_capture/<correspondence>`
- date and time is used as correspondence

## Json data over UDP
> [!NOTE]
> - Tested on Windows10

### build (C++)
#### Windows
```shell
cd udp_json_stream
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release
```
- Then, the **executible**s will be located at `udp_json_stream/build/Release`, i.e., `UdpJsonStreaming_listener.exe`.
#### Ubuntu Linux
(WIP)

### Usage
#### C++
- Build the project.
- Execute it as: 
  - Windows: `./UdpJsonStreaming_listener.exe <ip> <port>`
  - Linux: `./UdpJsonStreaming_listener <ip> <port>` (WIP)
- You can test it by running sample talker `./UdpJsonStreaming_talker[.exe] <ip> <port>` on another terminal.

#### Python
- No need to build anything.
- Simply call `python3 ./udp_json_stream/scripts/listener.py [--ip <ip>] [--port <port>]`
- You can test it by running sample talker `python3 ./udp_json_stream/scripts/sample_talker [--ip <ip>] [--port <port>]` on another terminal.