cmake_minimum_required(VERSION 3.1)
project(game)

# C flags
set(CMAKE_C_FLAGS "-g")
set(CMAKE_CXX_FLAGS "-g")
set(CMAKE_EXPORT_COMPILE_COMMANDS "1")

# Source files
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")
#set(SOURCES "${SRC_DIR}/*.c")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.h" "src/*.c")

# Executable definition and properties
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE "${SRC_DIR}")
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)

# stb
set(STB_DIR "${LIB_DIR}/stb")
target_include_directories(${PROJECT_NAME} PRIVATE "${STB_DIR}")

# cglm
set(CGLM_DIR "${LIB_DIR}/cglm")
add_subdirectory("${CGLM_DIR}" EXCLUDE_FROM_ALL)
target_link_libraries(${PROJECT_NAME} "cglm" cglm_headers) 


# glfw
set(GLFW_DIR "${LIB_DIR}/glfw")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
add_subdirectory("${GLFW_DIR}")
target_link_libraries(${PROJECT_NAME} "glfw" "${GLFW_LIBRARIES}")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLFW_DIR}/include")
target_compile_definitions(${PROJECT_NAME} PRIVATE "GLFW_INCLUDE_NONE")

# glad
set(GLAD_DIR "${LIB_DIR}/glad")
add_library("glad" "${GLAD_DIR}/src/gl.c")
target_include_directories("glad" PRIVATE "${GLAD_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLAD_DIR}/include")
target_link_libraries(${PROJECT_NAME} "glad" "${CMAKE_DL_LIBS}")

if(FALSE) # testing for imgui implementation
# cimgui
set(CIMGUI_DIR "${LIB_DIR}/cimgui")
set(CIMGUI_IMPL "${CIMGUI_DIR}/imgui/backends")

#add_subdirectory("${CIMGUI_DIR}")
#target_include_directories(${PROJECT_NAME} PRIVATE "${CIMGUI_DIR}")
#target_include_directories(${PROJECT_NAME} PRIVATE "${CIMGUI_DIR}/imgui")

file(GLOB 
	cimgui_files 
    "${CIMGUI_DIR}/*.cpp" 
    "${CIMGUI_DIR}/*.h" 
    "${CIMGUI_DIR}/imgui/*.cpp"
    "${CIMGUI_DIR}/imgui/*.h"
    "${CIMGUI_IMPL}/imgui_impl_glfw.cpp"
    "${CIMGUI_IMPL}/imgui_impl_opengl3.cpp"
    "${CIMGUI_IMPL}/*.h")
#target_include_directories(${PROJECT_NAME} PRIVATE "${CIMGUI_IMPL}")
#add_subdirectory("${CIMGUI_DIR}")
#add_subdirectory("${CIMGUI_DIR}/imgui")

include_directories(
    "${CIMGUI_DIR}"
    "${CIMGUI_DIR}/imgui"
    "${CIMGUI_IMPL}"
    )

add_library("cimgui" STATIC ${cimgui_files})
target_link_libraries("cimgui" PRIVATE "glfw")
target_include_directories("cimgui" PRIVATE "${CIMGUI_DIR}")
target_include_directories("cimgui" PRIVATE "${CIMGUI_DIR}/imgui")
target_include_directories("cimgui" PRIVATE "${CIMGUI_IMPL}")
target_compile_definitions("cimgui" PRIVATE
    IMGUI_IMPL_API=extern\ \"C\"
    IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include_directories("${CIMGUI_DIR}")
#include_directories("${CIMGUI_DIR}/imgui")
#include_directories("${CIMGUI_IMPL}")

target_compile_definitions(${PROJECT_NAME} PRIVATE IMGUI_IMPL_API=\ )

target_link_libraries(${PROJECT_NAME} "cimgui")

endif()
