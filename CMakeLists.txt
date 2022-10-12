cmake_minimum_required(VERSION 3.1)
project(game)

# C flags
set(CMAKE_C_FLAGS "-g")
set(CMAKE_CXX_FLAGS "-g -std=c++11 -O0")
set(CMAKE_EXPORT_COMPILE_COMMANDS "1")

# Source files
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")
#set(SOURCES "${SRC_DIR}/*.c")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.c" "src/*.cpp")

# Executable definition and properties
project(${PROJECT_NAME} C CXX)
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE "${SRC_DIR}")
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
target_compile_options(${PROJECT_NAME} PRIVATE -O0)

# stb
set(STB_DIR "${LIB_DIR}/stb")
target_include_directories(${PROJECT_NAME} PRIVATE "${STB_DIR}")

# cglm
set(CGLM_DIR "${LIB_DIR}/cglm")
add_subdirectory("${CGLM_DIR}" EXCLUDE_FROM_ALL)
target_link_libraries(${PROJECT_NAME} "cglm" cglm_headers) 
target_compile_definitions(${PROJECT_NAME} PRIVATE "CGLM_ALL_UNALIGNED")

# glfw
set(GLFW_DIR "${LIB_DIR}/glfw")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
add_subdirectory("${GLFW_DIR}")
target_link_libraries(${PROJECT_NAME} "glfw" "${GLFW_LIBRARIES}" "${CMAKE_DL_LIBS}")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLFW_DIR}/include")
target_compile_definitions(${PROJECT_NAME} PRIVATE "GLFW_INCLUDE_NONE")

# glad
set(GLAD_DIR "${LIB_DIR}/glad")
add_library("glad" "${GLAD_DIR}/src/gl.c")
target_include_directories("glad" PRIVATE "${GLAD_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLAD_DIR}/include")
target_link_libraries(${PROJECT_NAME} "glad" "${CMAKE_DL_LIBS}")

set(IMGUI_DIR "${LIB_DIR}/imgui")

add_library("imgui" "${IMGUI_DIR}/imgui.cpp"
                  "${IMGUI_DIR}/imgui_demo.cpp"
                  "${IMGUI_DIR}/imgui_draw.cpp"
                  "${IMGUI_DIR}/imgui_tables.cpp"
                  "${IMGUI_DIR}/imgui_widgets.cpp"
                  
                  "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
                  "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp")

target_include_directories("imgui" PRIVATE "${IMGUI_DIR}")
target_include_directories("imgui" PRIVATE "${IMGUI_DIR}/backends")
target_include_directories(${PROJECT_NAME} PRIVATE "${IMGUI_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE "${IMGUI_DIR}/backends")
target_link_libraries(${PROJECT_NAME} "imgui" "${CMAKE_DL_LIBS}")
target_link_libraries("imgui" "glfw" "${CMAKE_DL_LIBS}")

# Lua
find_package(Lua51 REQUIRED)
include_directories(/usr/local/include ${LUA_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${LUA_LIBRARIES})
