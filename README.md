![Sample](https://files.slindev.com/images/rayne_github.jpg)

# Rayne
## About
Rayne is a game engine for Windows, macOS, Linux and Android (and iOS shouldn't be missing much) written in C++. It currently has rendering implementations for D3D12, Vulkan and Metal.

Most functionality (like physics, path finding and UI) is added with optional modules. Not all included modules are fully working, some may not build at all. Modules can be found in the *Modules* directory.
Some of the working modules are:
* D3D12
* Vulkan
* Metal
* UI
* OpenAL
* Ogg
* PhysX
* Oculus (Requires the Oculus SDK to be downloaded and copied into the module folder!) - Only works with D3D12!
* Oculus Mobile (Requires the Oculus Mobile SDK to be downloaded and copied into the module folder!) - Only works with Vulkan!
* OpenVR
* ENet
* Recast
* Assimp

There are a couple of useful python scripts in the *Tools* directory. Most of these currently require Python 2.7.


## Required Software
* [CMake](https://cmake.org)
* [Python](https://www.python.org)
* [Ninja](https://ninja-build.org)


## Getting Started

Create a folder structure like the following (*Rayne* should be the directory containing this readme and everything else from this repo)
- root
  - Rayne
  - Games
    - GameName

1. Open a terminal and change the current directory to GameName.
2. Run ```python3 ../../Rayne/Tools/ProjectGenerator/CreateProject.py``` (This is the only one that needs python3... the others need 2.7...)
*Template Name* has to be *base-vr* at the moment (it also works without VR, but handles some additional setup needed for VR), everything else can be chosen however you want.
3. Run ```python ../../Rayne/Tools/BuildHelper/CreateBuildProject.py build-config.json platform independent``` replace *platform* with either *windows* (only works on windows), *macos* (only works on macOS), *linux* (only works on linux) or *android* (should work on any host system, only really tested on macOS though).
This will create a new *Builds* directory with another directory for the platform inside. For windows this will contain a Visual Studio solution, for macOS it will be an XCode workspace, on linux it will be ninja makefiles and for android it will be a gradle project that can be imported into Android Studio.
4. Before you can successfully run the project, go to *Assets/Shaders* and run *CompileShaders.bat* / *CompileShaders.sh*.
5. Then build and run and everything should work.


## Screenshots
![Vehicle Construction Kit](https://files.slindev.com/images/rayne_github_vck.jpg)
![Concealed](https://files.slindev.com/images/rayne_github_concealed.jpg)
![Project Z](https://files.slindev.com/images/rayne_github_projectz.jpg)
![Blobby Tennis](https://files.slindev.com/images/rayne_github_blobbytennis.jpg)
![Ski Jump](https://files.slindev.com/images/rayne_github_skijump.jpg)