# ![Goodsack](docs/public/gsk_banner.png?raw=true "Hero")
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://github.com/gabekz/GoodsackEngine/blob/nov23_buildsys/LICENSE.txt)
[![Build/CI](https://github.com/gabekz/GoodsackEngine/actions/workflows/runner_root.yml/badge.svg?event=push)](https://github.com/gabekz/GoodsackEngine/actions/workflows/runner_root.yml)

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

## Example usage

ECS components created here: https://github.com/gabekz/GoodsackEngine/goodsack/gsk_data/components.json

```C
/*--------------------------------------------------------------------------
  
   Example showing how one might go about creating a basic scene.

   As of December 2023, the symbols are not accurate. To make this
   look cleaner, I have modified the code to show what the function
   names are expected to become. See 'demo_hot' in the 'demos/' directory
   for an accurate depiction.
   
 --------------------------------------------------------------------------*/

static void
CreateAndLoadMyScene(gsk_ECS *ecs_handle, gsk_Renderer *renderer)
{
    //------------------------------------------------
    // Scene setup

    ecs_handle = gsk_renderer_active_scene(renderer, 2); // set runtime to scene index 2
    gsk_set_active_scene_skybox(renderer, SKYBOX_MAIN);

    //------------------------------------------------
    // Create resources (lazy-load at scene startup)

    gsk_Model *model_sponza=
      gsk_model_load_from_file("data://models/sponza.glb", TRUE, TRUE);

    //------------------------------------------------
    // Create camera entity

    gsk_Entity ent_camera = gsk_ecs_new(ecs_handle);
    gsk_ecs_add(ent_camera,
                C_CAMERA,
                ((struct ComponentCamera) {
                    .axis_up      = {0.0f, 1.0f, 0.0f},
                    .render_layer = 0,
                }));
    gsk_ecs_add(ent_camera,
                C_CAMERALOOK,
                ((struct ComponentCameraLook) {
                    .sensitivity = 1.0f,
                }));
    gsk_ecs_add(ent_camera,
                C_CAMERAMOVEMENT,
                ((struct ComponentCameraMovement) {
                    .speed = 2.5f,
                }));
    gsk_ecs_add(ent_camera,
                C_TRANSFORM,
                ((struct ComponentTransform) {
                    .position = {0, 10, 0},
                }));

    //------------------------------------------------
    // Create sponza entity

    gsk_Entity e_sponza = gsk_ecs_new(ecs_handle);
    gsk_ecs_add(e_sponza,
                C_TRANSFORM,
                ((struct ComponentTransform) {
                    .position = {0.0f, -1.5f, 0.0f},
                    .scale    = {0.001f, 0.001f, 0.001f},
                }));
    gsk_ecs_add(e_sponza,
                C_MODEL,
                ((struct ComponentModel) {
                    .pModel   = model_sponza,
                }));
}

//------------------------------------------------
// Program entry

int
main(void)
{
    gsk_runtime_setup((_PROJ_DIR_DATA "/"), argc, argv);

    gsk_ECS *p_ecs             = gsk_runtime_get_ecs();
    gsk_Renderer *p_renderer   = gsk_runtime_get_renderer();

    CreateAndLoadMyScene(p_ecs, p_renderer);
    gsk_runtime_set_scene(2); // Set the active scene. Note, already done in CreateAndLoadMyScene()

    gsk_runtime_loop();

    return 0;
}

```

> # ![Sponza](docs/public/sponza.png?raw=true "Sponza Example")
> output

---

## Build

## [Linux] Building with CMake

Building for Linux is fairly straight-forward. Navigating to the root directory, we can run the following:

```
mkdir build && cd build
cmake ../CMakeLists.txt
cmake --build .
```
