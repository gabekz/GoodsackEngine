cmake_minimum_required(VERSION 3.1)
project(below-eng)

set(GAME_EXE_NAME "game")
set(TEST_EXE_NAME "testRunner")

#################################################################
# Compiler flags
#################################################################
set(CMAKE_C_FLAGS "-g")
set(CMAKE_CXX_FLAGS "-g -std=c++14 -O0")
set(CMAKE_EXPORT_COMPILE_COMMANDS "1")

#################################################################
# Source files
#################################################################
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")

#file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${SUB_FOLDERS}/*.c" "${SUB_FOLDERS}/*.cpp")
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${SRC_DIR}/*.c" "${SRC_DIR}/*.cpp")
file(GLOB ${TEST_SOURCES} CONFIGURE_DEPENDS "")

set(TEST_SUBDIR_NAME ".*__tests__.*")
# For each item in our list
foreach (item ${SOURCES})
    # Check if our items matches our regular expression
    if ("${item}" MATCHES ${TEST_SUBDIR_NAME})
        # Remove current item from our list
        list (REMOVE_ITEM SOURCES "${item}")
        list (APPEND TEST_SOURCES "${item}")
    endif ("${item}" MATCHES ${TEST_SUBDIR_NAME})
endforeach(item)

#################################################################
# Executable definition and properties
#################################################################
project(${PROJECT_NAME} C CXX)

add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC "${SRC_DIR}")

add_executable(${GAME_EXE_NAME} "${SRC_DIR}/main.cpp")
target_link_libraries(${GAME_EXE_NAME} ${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
target_compile_options(${PROJECT_NAME} PUBLIC -O0)

#################################################################
# Library: STB Image
#################################################################
set(STB_DIR "${LIB_DIR}/stb")
target_include_directories(${PROJECT_NAME} PRIVATE "${STB_DIR}")

#################################################################
# Library: CGLM
#################################################################
set(CGLM_DIR "${LIB_DIR}/cglm")
add_subdirectory("${CGLM_DIR}" EXCLUDE_FROM_ALL)
target_link_libraries(${PROJECT_NAME} "cglm" cglm_headers) 
target_compile_definitions(${PROJECT_NAME} PRIVATE "CGLM_ALL_UNALIGNED")

#################################################################
# Library: GLFW
#################################################################
set(GLFW_DIR "${LIB_DIR}/glfw")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
add_subdirectory("${GLFW_DIR}")
target_link_libraries(${PROJECT_NAME} "glfw" "${GLFW_LIBRARIES}" "${CMAKE_DL_LIBS}")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLFW_DIR}/include")
target_compile_definitions(${PROJECT_NAME} PRIVATE "GLFW_INCLUDE_NONE")

#################################################################
# Library: GLAD
#################################################################
set(GLAD_DIR "${LIB_DIR}/glad")
add_library("glad" "${GLAD_DIR}/src/gl.c")
target_include_directories("glad" PUBLIC "${GLAD_DIR}/include")
target_include_directories(${PROJECT_NAME} PUBLIC "${GLAD_DIR}/include")
target_link_libraries(${PROJECT_NAME} "glad" "${CMAKE_DL_LIBS}")

#################################################################
# Library: ImGui
#################################################################
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

#################################################################
# Library: Lua
#################################################################
find_package(Lua51 REQUIRED)
include_directories(/usr/local/include ${LUA_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${LUA_LIBRARIES})

#################################################################
# Library: JSON
#################################################################
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_INSTALL OFF CACHE INTERNAL "")
add_subdirectory("${LIB_DIR}/json")
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)

#################################################################
# Library: Google Test
#################################################################
set(GTEST_DIR "${LIB_DIR}/googletest") 
add_subdirectory(${GTEST_DIR})
enable_testing()
target_include_directories(${PROJECT_NAME} PRIVATE
    "${GTEST_DIR}" 
    "${GTEST_DIR}/googletest"
    "${GTEST_DIR}/googletest/include"
    "${GTEST_DIR}/googlemock"
    "${GTEST_DIR}/googlemock/include"
    )
#target_include_directories(${PROJECT_NAME} PRIVATE "${GTEST_DIR}/include")
#target_include_directories(${PROJECT_NAME} PRIVATE "${GTEST_DIR}")

#################################################################
# UNIT TESTING
#################################################################
add_executable(${TEST_EXE_NAME} ${TEST_SOURCES} )
# Link test executable against gtest & gtest_main
target_link_libraries(${TEST_EXE_NAME} gtest gtest_main)
target_link_libraries(${TEST_EXE_NAME} ${PROJECT_NAME}) 
target_include_directories(${TEST_EXE_NAME} PRIVATE "${SRC_DIR}")
add_test(${TEST_EXE_NAME} ${TEST_EXE_NAME})
