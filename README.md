![Alt text](docs/public/gsk_banner.png?raw=true "Hero")

[![Build](https://github.com/gabekz/GoodsackEngine/actions/workflows/runner_root.yml/badge.svg?branch=master)](https://github.com/gabekz/GoodsackEngine/actions/workflows/runner_root.yml)

---

## Why?
The purpose of this engine is to make games *differently*. I don't want to rely on a GUI for constructing almost every aspect of a game, because I ultimately
believe that it slows us down as developers. GUI programs for tasks involving Level Design may be a very good use-case, but I shouldn't need the mouse to set
the properties of an entity.

Games are difficult to make. By making it easier for those who have no experience with Game Development, tools that rely on GUI do make the barrier of entry a
lot shorter for newcomers. However, I want to make it easier for those who have been in the industry for a long time.

This project started as a simple exercise for graphics rendering with OpenGL - now it has evolved into something much more. My vision for this engine is particularly
inspired by the [Valve Source Engine](https://developer.valvesoftware.com/wiki/Source). I want to create an engine that is built for developers, with the added
versatility and integral foundation for 3rd-party mod support.

> [!WARNING]
The Goodsack engine is still in the early stages of development. Because building blocks of this project are
most likely going to change significantly, I would highly encourage you to **seek other venues** for a professional project.
There are so many great projects right now, such as [Godot](https://github.com/godotengine/godot) and [Bevy](https://github.com/bevyengine/bevy) just to name a couple.
Feel free to stick around if you love game engine development!

---

## Features
- Custom Rendering Pipelines - PBR out of the box
- 2D & 3D Graphics Rendering
- Cross Platform - Windows and Linux supported
- Lua scripting API
- Spatial audio
- Dynamic Physics
- Scene/Level Management

---

# Installation

## [Linux] Building with CMake

Building with CMake is pretty easy here. Navigating to the root directory, we can run the following:

```
mkdir build && cd build
cmake ../CMakeLists.txt
cmake --build .
```
