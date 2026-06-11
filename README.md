# Real-Time Atmospheric Scattering Sky Renderer

## Overview

This project is a real-time OpenGL sky rendering application that demonstrates atmospheric scattering effects.

It renders a procedural sky around the camera using a sky cube and shader-based atmosphere parameters. The application allows the user to interactively adjust the sun elevation, atmospheric density, ray-marching step count, sun size, and aerosol scattering coefficient while observing the result in real time.

This project was developed for a Real-Time Rendering assignment using C++, OpenGL, GLFW, GLAD, GLM, and GLSL shaders.

## Demo Video

A short demonstration video of the project is available on YouTube:

[Watch the demo on YouTube](https://youtu.be/6Kb5csYMgCE)

## Features

* Real-time procedural sky rendering
* OpenGL 4.5 Core Profile rendering pipeline
* Sky cube generated directly in C++
* Shader-based atmospheric scattering
* Interactive first-person camera movement
* Mouse-look camera control
* Real-time adjustment of atmosphere parameters
* FPS and parameter values displayed in the window title
* Uniform Buffer Objects for camera, lighting, sky transform, and atmosphere data

## Project Structure

```text
RTR_Code/
│
├── main.cpp                 # Main application source code
├── glad.c                   # GLAD OpenGL loader source
│
├── vertSky.vert             # Sky vertex shader
├── fragSky.frag             # Sky fragment shader
│
├── vertex.vert              # Additional / experimental vertex shader
├── fragment.frag            # Additional / experimental fragment shader
├── vertShadow.vert          # Additional / experimental shadow vertex shader
│
├── RTR.sln                  # Visual Studio solution file
├── RTR.vcxproj              # Visual Studio project file
├── RTR.vcxproj.filters      # Visual Studio project filters
│
└── .gitignore
```

The current application mainly loads and uses:

```text
vertSky.vert
fragSky.frag
```

These shader files must be available in the program's working directory when running the project.

## Requirements

* Windows
* Visual Studio 2022 or later
* C++ compiler with C++17 support
* OpenGL 4.5 compatible GPU and graphics driver
* GLFW
* GLAD
* GLM
* stb_image / stb_image_write

## How to Build and Run

### 1. Clone the repository

```bash
git clone https://github.com/Vivianyoo/Real-Time-Atmospheric-Scattering-Sky-Renderer.git
cd Real-Time-Atmospheric-Scattering-Sky-Renderer
```

### 2. Open the project

Open the Visual Studio solution file:

```text
RTR.sln
```

### 3. Check the working directory

The program loads shader files using relative paths, for example:

```cpp
LoadTextFile("vertSky.vert");
LoadTextFile("fragSky.frag");
```

Therefore, make sure the Visual Studio working directory is set to the folder containing these shader files.

Recommended Visual Studio setting:

```text
Project Properties > Configuration Properties > Debugging > Working Directory
```

Set it to:

```text
$(ProjectDir)
```

### 4. Build and run

Build the project in Visual Studio and run it using:

```text
Local Windows Debugger
```

If everything is set correctly, a window should appear showing the real-time atmospheric sky.

## Controls

| Input          | Function                                |
| -------------- | --------------------------------------- |
| Mouse movement | Look around                             |
| W              | Move forward                            |
| S              | Move backward                           |
| A              | Move left                               |
| D              | Move right                              |
| Up Arrow       | Increase sun elevation                  |
| Down Arrow     | Decrease sun elevation                  |
| 1              | Decrease atmospheric density            |
| 2              | Increase atmospheric density            |
| 3              | Decrease ray-marching step count        |
| 4              | Increase ray-marching step count        |
| 7              | Decrease sun size                       |
| 8              | Increase sun size                       |
| 9              | Decrease aerosol scattering coefficient |
| 0              | Increase aerosol scattering coefficient |
| R              | Reset all atmosphere parameters         |

To exit the program, close the window or use `Alt + F4`.

## Adjustable Parameters

The application exposes several real-time atmosphere parameters:

| Parameter     | Description                                      | Default Value |
| ------------- | ------------------------------------------------ | ------------- |
| Sun Elevation | Controls the vertical angle of the sun           | 25 degrees    |
| Density       | Controls the strength of atmospheric density     | 1.0           |
| Steps         | Controls the number of atmosphere sampling steps | 24            |
| Sun Size      | Controls the apparent size of the sun disk       | 0.0093        |
| betaA         | Controls the aerosol scattering coefficient      | 0.000012      |
| Thickness     | Atmosphere thickness value used by the shader    | 80000.0       |

The window title displays the current FPS and parameter values during runtime.

## Implementation Notes

The sky is rendered using a cube that follows the camera position. This makes the sky appear infinitely far away from the viewer.

The project uses several Uniform Buffer Objects:

* `CameraBlock`
* `LightingBlock`
* `SkyTransformBlock`
* `AtmosphereBlock`

These blocks pass camera matrices, sun direction, sky transform data, and atmosphere parameters from C++ to the GLSL shaders.

The sun direction is calculated from the sun elevation angle:

```cpp
g_lightUBO.sunDir = glm::normalize(glm::vec3(
    cos(sunAngleRad),
    sin(sunAngleRad),
    0.0f
));
```

The sky intensity is also updated dynamically according to the sun elevation.

## Troubleshooting

### Shader files cannot be opened

If the console prints messages such as:

```text
Failed to open file: vertSky.vert
Failed to open file: fragSky.frag
```

check that the shader files are in the program's working directory.

### The window opens but the sky does not render correctly

Possible causes:

* Shader compilation failed
* Shader files are missing
* The working directory is incorrect
* The GPU driver does not support OpenGL 4.5

Check the console output for shader compile or program link errors.

### GLAD fails to initialize

If the console prints:

```text
Failed to initialize GLAD.
```

make sure the OpenGL context is created correctly and your graphics driver is up to date.

## Notes

* Build output folders such as `x64/`, `Debug/`, and `Release/` are ignored.
* User-specific Visual Studio files such as `.user` files are ignored.
* The shader files must remain in the correct working directory for the program to run properly.
* The project is intended for real-time rendering learning and demonstration.

## Author

Jiayun Zhang / Vivian
