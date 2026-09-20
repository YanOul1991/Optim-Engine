# Rendering Engine with Vulkan

A personal project to learn 3D rendering using the Vulkan API with dynamic rendering, featuring Slang as the default shader language.

This project is an improved and re-architecture version of a previous [capstone project](link to previous repo).

<br>

# Dependencies & Requirements

### CMake  
This project is built and tested with CMake version `4.2.3`.

### vcpkg
Open-source libraries are managed via vcpkg integrated with CMake.

Current dependencies:
* SDL3
* fmt
* Slang

### Git
Required for vcpkg dependency management.

### Vulkan SDK
The Vulkan SDK can be downloaded from the official [LunarG website](https://vulkan.lunarg.com/sdk/home).

Tested SDK version: `1.4.350.0`

<br>

# Building the Project

To build the project using CMake presets, run:

```bash
# Configure and build with a preset (debug or release)
cmake --preset debug
cmake --build --preset debug
```

The compiled binaries will be located in the a `output/{preset}` directory.

<br>

# Technical Specifications
* Language Standard: C++20
* Graphics API: Vulkan (Dynamic Rendering / Vulkan 1.4)
* Shader Language: Slang
* Target Platforms: Windows & Linux
