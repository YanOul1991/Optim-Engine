# RENDERING ENGINE WITH VULKAN
Small personal project for learning 3D rendering utilizing the Vulkan API.

[>>> Vulkan Tutorial Link <<<](https://docs.vulkan.org/tutorial/latest/00_Introduction.html)

<br>

# Dependencies & Requirements

This project contains the following requierments:
- CMake
- vcpkg
- Vulkan SDK
- Git

### VCPKG

Open Sources libraries such as SDL3 dependencies are managed with vcpkg 
with CMake.

This avoids needing to dedicate a third party directory taking space and 
prevents manually rebuilding libraries with the correct settings when
switching to a new machine or operating system.

### Vulkan SDK

For vulkan since it is not managed with vcpkg it must be installed from [the LunarG website](https://vulkan.lunarg.com/sdk/home).

This project used the SDK version `1.4.350.0`.

<br><br>

# Building the project

To build the project, simply run the CMake setup commands with one the 
following presets :
  * debug
  * release

The generated files should be in the `output/{preset}` directory.
