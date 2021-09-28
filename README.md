# EnvInteraction
Cross platform c++ library for interaction with window environment. There is built-in real time window capturing and event sending

## Goal
Goal of this project is to build easy embeddable cross platform library for interacting with graphical interface from c++

Currently built and tested with Linux and Windows

 - [x] Window capturing
 - [x] Recovering images to disk
    - [x] Programmable limitation
    - [x] Multithreading
 - [x] Inputs
    - [x] Mouse events
    - [x] Keyboard events

Potentially this project will be used in computer vision one to interact with real time frame changing and immediately reacting to this

## Requirement

- Linux: Xinerama Xtst Xfixes
- Windows: dwmapi

## Build

There is premake extension file for premake5 build system. Just call premake.exe with specified target and compile generated project. More details are [there](https://premake.github.io/docs/) 

```
Ubuntu
$> git clone https://github.com/DeinsOne/Envi.git
$> cd Envi
$> sudo apt-get install libxinerama-dev libxtst-dev libxcb-xfixes0 
$> premake5 gmake
$> make config=release

Windows
$> premake5.exe vs2019
$> ../MSBuild.exe Environment.sln
```