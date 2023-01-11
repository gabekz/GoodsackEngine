# Below Game Engine
[![Build](https://github.com/gabekz/gl-renderer/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/gabekz/gl-renderer/actions/workflows/build.yml)
[![Lint](https://github.com/gabekz/gl-renderer/actions/workflows/lint.yml/badge.svg)](https://github.com/gabekz/gl-renderer/actions/workflows/lint.yml)

Real-time render engine, built with C/C++. Feature complete with ECS, Lua scripting and custom physics! 

![Alt text](docs/public/engine.png?raw=true "Title")

# Features
- Physically-Based Rendering Pipeline
- Forward & Deferred Rendering
- Lua Support
- Entity-Component-System (ECS)
- OpenAL Audio
- OpenGL and Vulkan GPU Backends
- Dynamic Physics (Rigidbody simulation)
- Point-light Shadows
- Scene Manager

# Supported Platforms
![Alt text](docs/public/win-mac-lux.png?raw=true "Supported Platformsz")

---

# Installation

## [Linux] Building with CMake

Building with CMake is pretty easy here. Navigating to the root directory, we can run the following:

```
mkdir build && cd build
cmake ../CMakeLists.txt
cmake --build .
```

For help, use `beloweng -h` or `-beloweng --help`.

# Usage

## Creating a new Project


---
## Dependencies
- glfw (Window Manager)
- glad (OpenGL Loader)
- cglm (Math)
- lua
- nlohmann_json
- Google Test (unit testing framework)
- ImGui (Fantastic Debug-GUI Library)
- stb_img (Image Loader)



