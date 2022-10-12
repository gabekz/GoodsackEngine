#include <iostream>
#include <string>

#include <util/sysdefs.h>
#include <util/maths.h>

#include <core/ecs.h>
#include <core/renderer.h>

extern "C" {
#include <core/lighting.h>

#include <components/transform.h>
#include <components/camera.h>
#include <components/mesh.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include <debug/debugview.h>

#define texture_create_d(x) texture_create(x, GL_SRGB_ALPHA, true, 16)
#define texture_create_n(x) texture_create(x, GL_RGB, false, 1)

static int CheckLua(lua_State *L, int r) {
    if(r != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        printf("[Lua Error]: %s", err);
        return 0;
    }
    return 1;
}

static void LuaTest() {

    struct Player {
        const char* name;
        int age;
    };

    struct Player player = {"Undefined", 69};

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    if (CheckLua(L, luaL_dofile(L, "../test.lua"))) {
        lua_getglobal(L, "a");
        if(lua_isnumber(L, -1)) {
            printf("%f", (float)lua_tonumber(L, -1));
        }

        lua_getglobal(L, "PlayerName");
        if(lua_isstring(L, -1)) {
            player.name = lua_tostring(L, -1);
        }

        lua_getglobal(L, "Player");
        if(lua_istable(L, -1)) {
            lua_pushstring(L, "Name");
            lua_gettable(L, -2);
            player.name = lua_tostring(L, -1);
            lua_pop(L, 1);
        }

        lua_getglobal(L, "Add");
        if(lua_isfunction(L, -1)) {
            lua_pushnumber(L, 52);
            lua_pushnumber(L, 12);

            if(CheckLua(L, lua_pcall(L, 2, 1, 0))) {
                printf("\nFrom C, Lua Add(): %f.", lua_tonumber(L, -1));
            }

        }
    }

    printf("\nPlayer name: %s", player.name);
    printf("\n");

    lua_close(L);
}

int main(int argc, char *argv[]) {
    if(argc > 1) {
        for(int i = 0; i < argc; i++) {

            if(std::string(argv[i]) == "--debug") {
                // do something
            }
        }
    }

// Call lua foundation-test
    LuaTest();

// Initialize Renderer
    Renderer *renderer = renderer_init();
    int winWidth = renderer->windowWidth;
    int winHeight = renderer->windowHeight;

// Initialize ECS
    ECS *ecs;
    ecs = renderer_active_scene(renderer, 0);

// Lighting information
    vec3 lightPos     = {1.0f, 2.8f, -0.2f};
    vec4 lightColor   = {1.0f, 1.0f, 1.0f, 1.0f};

// UBO Lighting
    lighting_initialize(lightPos, lightColor);

    Texture *texDefSpec =
        texture_create_n("../res/textures/defaults/specular.png");
    Texture *texDefNorm =
        texture_create_n("../res/textures/defaults/normal.png");

    Texture *texEarthDiff = texture_create_d("../res/textures/earth/diffuse.png");
    Texture *texEarthNorm = texture_create_n("../res/textures/earth/normal.png");

    Texture *texContDiff = texture_create_d("../res/textures/container/diffuse.png");
    Texture *texContSpec = texture_create_n("../res/textures/container/specular.png");

    Texture *texBrickDiff =
        texture_create_d("../res/textures/brickwall/diffuse.png");
    Texture *texBrickNorm =
        texture_create_n("../res/textures/brickwall/normal.png");

    Material *matSuzanne =
        material_create(NULL, "../res/shaders/lit-diffuse.shader", 3,
        texEarthDiff, texEarthNorm, texDefSpec);

// Create the Camera, containing starting-position and up-axis coords.
    Entity camera = ecs_new(ecs);
    ecs_add(camera, C_CAMERA, ((struct ComponentCamera) {
        .position = {0.0f, 0.0f, 2.0f},
        .axisUp   = {0.0f, 1.0f, 0.0f},
        .speed    = 0.05f,
    }));

    Entity suzanneObject = ecs_new(ecs);
    ecs_add(suzanneObject, C_TRANSFORM);
    ecs_add(suzanneObject, C_MESH, ((struct ComponentMesh) {
        .material = matSuzanne,
        .modelPath = "../res/models/sphere.obj",
        .properties = {
            .drawMode = DRAW_ARRAYS,
            .cullMode = CULL_CW | CULL_FORWARD,
        }
    }));

/*------------------------------------------- 
|   Scene #2
*/
    ecs = renderer_active_scene(renderer, 1);

    Material *matFloor = 
        material_create(NULL, "../res/shaders/lit-diffuse.shader",
        3, texBrickDiff, texBrickNorm, texDefSpec); 
    Material *matBox = 
        material_create(NULL, "../res/shaders/lit-diffuse.shader",
        3, texContDiff, texDefNorm, texContSpec); 

    Entity camera2 = ecs_new(ecs);
    ecs_add(camera2, C_CAMERA, ((struct ComponentCamera) {
        .position = {0.0f, 0.0f, 2.0f},
        .axisUp   = {0.0f, 1.0f, 0.0f},
        .speed    = 0.05f,
    }));

    Entity floorEntity = ecs_new(ecs);
    ecs_add(floorEntity, C_TRANSFORM, ((struct ComponentTransform) {
            .position = {0.0f, -0.3f, 0.0f},
            .scale = {10.0f, 10.0f, 10.0f},
    }));
    ecs_add(floorEntity, C_MESH, ((struct ComponentMesh) {
        .material = matFloor,
        .modelPath = "../res/models/plane.obj",
        .properties = {
            .drawMode = DRAW_ARRAYS,
            .cullMode = CULL_CW | CULL_FORWARD,
        }
    }));

    Entity boxEntity = ecs_new(ecs);
    ecs_add(boxEntity , C_TRANSFORM, ((struct ComponentTransform) {
        .position = {0.0f, -0.085f, 0.0f},
    }));
    ecs_add(boxEntity, C_MESH, ((struct ComponentMesh) {
        .material = matBox,
        .modelPath = "../res/models/cube-test.obj",
        .properties = {
            .drawMode = DRAW_ARRAYS,
            .cullMode = CULL_CW | CULL_FORWARD,
        }
    }));

/*------------------------------------------- 
|   Scene #3
*/
    ecs = renderer_active_scene(renderer, 2);
    Entity camera3 = ecs_new(ecs);
    ecs_add(camera3, C_CAMERA, ((struct ComponentCamera) {
        .position = {0.0f, 0.0f, 2.0f},
        .axisUp   = {0.0f, 1.0f, 0.0f},
        .speed    = 0.05f,
    }));

    Entity sphereEntity = ecs_new(ecs);
    ecs_add(sphereEntity, C_TRANSFORM, ((struct ComponentTransform) {
        .position = {0.0f, 0.0f, 0.0f},
    }));

    Texture *texGraniteAlbedo =
        texture_create_d("../res/textures/pbr/granite/albedo.png");
    Texture *texGraniteNormal =
        texture_create_n("../res/textures/pbr/granite/normal.png");
    Texture *texGraniteMetallic =
        texture_create_n("../res/textures/pbr/granite/metallic.png");
    Texture *texGraniteSpecular =
        texture_create_n("../res/textures/pbr/granite/roughness.png");
    Texture *texGraniteAo = 
        texture_create_n("../res/textures/pbr/granite/ao.png");
    Material *matGranite = 
        material_create(NULL, "../res/shaders/pbr.shader",
        5,
        texGraniteAlbedo, texGraniteNormal, 
        texGraniteMetallic, texGraniteSpecular, texGraniteAo 
    );

    Texture *texPbrAlbedo =
        texture_create_d("../res/textures/pbr/rust/albedo.png");
    Texture *texPbrNormal =
        texture_create_n("../res/textures/pbr/rust/normal.png");
    Texture *texPbrMetallic =
        texture_create_n("../res/textures/pbr/rust/metallic.png");
    Texture *texPbrSpecular =
        texture_create_n("../res/textures/pbr/rust/roughness.png");
    Texture *texPbrAo = 
        texture_create_n("../res/textures/defaults/white.png");
    Material *matRust = 
        material_create(NULL, "../res/shaders/pbr.shader",
        5,
        texPbrAlbedo, texPbrNormal,
        texPbrMetallic, texPbrSpecular, texPbrAo
    );

    ecs_add(sphereEntity, C_MESH, ((struct ComponentMesh) {
        .material = matGranite,
        .modelPath = "../res/models/sphere.obj",
        .properties = {
            .drawMode = DRAW_ARRAYS,
            .cullMode = CULL_CW | CULL_FORWARD,
        }
    }));
    Entity floorEntity2 = ecs_new(ecs);
    ecs_add(floorEntity2, C_TRANSFORM, ((struct ComponentTransform) {
            .position = {0.0f, -0.3f, 0.0f},
            .scale = {10.0f, 10.0f, 10.0f},
    }));
    ecs_add(floorEntity2, C_MESH, ((struct ComponentMesh) {
        .material = matFloor,
        .modelPath = "../res/models/plane.obj",
        .properties = {
            .drawMode = DRAW_ARRAYS,
            .cullMode = CULL_CW | CULL_FORWARD,
        }
    }));
    
// TESTING
    static bool showWindow = true;
    DebugGui *debugGui = new DebugGui(renderer);

/* Render loop */
    renderer_active_scene(renderer, 2);

    renderer_start(renderer); // Initialization for the render loop
    while(!glfwWindowShouldClose(renderer->window)) {

        renderer_tick(renderer);

        debugGui->Render();

        glfwSwapBuffers(renderer->window); // we need to swap.
    }

// Cleanup
    delete(debugGui);

    glfwDestroyWindow(renderer->window);
    free(renderer);
    free(ecs);
    glfwTerminate();

    return 0;
}
