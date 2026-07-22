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

To be able to use vkpkg it must be installed first following the
[vcpkg installation steps](https://learn.microsoft.com/en-us/vcpkg/get_started/overview#get-started-with-vcpkg).


### Vulkan SDK

For vulkan since it is not managed with vcpkg it must be installed from [the LunarG website](https://vulkan.lunarg.com/sdk/home).

This project used the SDK version `1.4.350.0`.

<br><br>

# Building the project

To build the project, first run the CMake setup command:

```bash
cmake -B build -S . --preset default
```

The simply run:

```bash
cmake --build build
```

The generated files should be in an `output/` directory.

Finally, to run the program itself simply run:
```bash
 ./output/optimengine
```