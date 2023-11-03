#################################################################
# "Below" Game Engine - CMake Configuration
# https://www.github.com/gabekz/gl-renderer

cmake_minimum_required(VERSION 3.20)
project(
    GoodsackEngine
    VERSION 0.1.0.0
    DESCRIPTION "Realtime 3D Rendering Engine"
    HOMEPAGE_URL "github.com/gabekz/GoodsackEngine")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_COLOR_MAKEFILE ON)
include(colors)
define_colors()

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#################################################################
# Release Targets (not multi-configuration friendly..)
# TODO: Change from CMAKE_BUILD_TYPE to something project-specific
#################################################################
set(CMAKE_BUILD_TYPE "Release") # Override the following checks
set(default_build_type "Debug")
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
  set(default_build_type "Debug")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' \
as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

message("Default build Type: ${default_build_type}")
message("CMake Build Type: ${CMAKE_BUILD_TYPE}")
message("Configuration Type: ${CMAKE_BUILD_TYPE}")

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    message("release mode")
endif()

#################################################################
# Compiler flags
#################################################################
set(CMAKE_EXPORT_COMPILE_COMMANDS "1")
set(CMAKE_C_STANDARD 17)
#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD_COMPILE_OPTION "-std:c++latest")
#set(CMAKE_CXX_EXTENSION_COMPILE_OPTION "-std:c++latest")

if(MSVC)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if (MSVC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.29.30129 AND CMAKE_VERSION VERSION_GREATER 3.20.3)
    # this change happened in CMake 3.20.4
    set(CMAKE_CXX_STANDARD 23) # /std:c++latest - unlocks the non stable cpp20 features. For new 16.11 versions
else ()
    set(CMAKE_CXX_STANDARD 20) # /std:c++latest for msvc and -std=c++20 for everyone else.
endif ()

if(UNIX)
    set(CMAKE_C_FLAGS "-g -O0")
    set(CMAKE_CXX_FLAGS "-g -std=c++14 -O0")
    set(CMAKE_CXX_STANDARD 14) # /std:c++latest for msvc and -std=c++20 for everyone else.

    set(CMAKE_CXX_FLAGS_RELEASE "-g -std=c++14 -O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -std=c++14 -O0")
endif()


set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)

#################################################################
# Source files
#################################################################
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(RES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")
set(API_DIR "${SRC_DIR}/api/llib")

set(TEST_SUBDIR_REGEX ".*__tests__.*")
# set(API_SUBDIR_REGEX ".*__api__.*")

# All Sources
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${SRC_DIR}/*.c"
    "${SRC_DIR}/*.cpp")

# Lua API-only sources
file(GLOB_RECURSE API_SOURCES CONFIGURE_DEPENDS
    "${API_DIR}/common.h"
    "${API_DIR}/llib_goodsack.c"
    "${API_DIR}/llib_vector.c"
    "${API_DIR}/goodsack/llib_time.c"
    "${API_DIR}/goodsack/llib_input.cpp")

# For each item in our list
foreach (item ${SOURCES})
    # Check if our items matches our regular expression
    if ("${item}" MATCHES ${TEST_SUBDIR_REGEX})
        # Remove current item from our list
        list (REMOVE_ITEM SOURCES "${item}")
        list (APPEND TEST_SOURCES "${item}")
    endif ()
endforeach()

######## EXECUTABLES ############################################

option(GOODSACK_EXAMPLES "Build eaxmple projects" ON)
option(GOODSACK_TEST "Build the TestRunner executable" ON)
option(GOODSACK_TEST_COVERAGE "Collect test coverage files" OFF)
option(GOODSACK_DEV "Development Mode" OFF)
option(GOODSACK_API_OMIT_PREFIX "Prefix" OFF)

add_subdirectory(thirdparty)

#################################################################
# GoodsackEngine Runtime Library
#################################################################
add_library(GoodsackEngine_runtime SHARED ${SOURCES})

set_target_properties(GoodsackEngine_runtime PROPERTIES
    if(GOODSACK_API_OMIT_PREFIX)
        PREFIX ""
    endif(GOODSACK_API_OMIT_PREFIX)
    FOLDER "GoodsackEngine"
    DESCRIPTION "GoodsackEngine Runtime Library"
)

target_include_directories(GoodsackEngine_runtime PUBLIC ${SRC_DIR} ${PROJECT_BINARY_DIR})

target_compile_options(GoodsackEngine_runtime PUBLIC ${PROJECT_EXE_OPTIONS})

target_link_libraries(GoodsackEngine_runtime PUBLIC
    ThirdParty_cglm
    ThirdParty_glfw
    ThirdParty_glad
    ThirdParty_vulkan
    ThirdParty_imgui
    ThirdParty_lua
    ThirdParty_alsoft
)

target_link_libraries(GoodsackEngine_runtime PRIVATE 
    ThirdParty_stb
    ThirdParty_json
    ThirdParty_cgltf
)

#if(GOODSACK_TEST)
#    target_link_libraries(GoodsackEngine_runtime PRIVATE ThirdParty_gtest)
#endif()

#################################################################
# GoodsackEngine Lua-API Library
#################################################################
add_library(GoodsackEngine_api SHARED ${API_SOURCES})
set_target_properties(GoodsackEngine_api PROPERTIES
    PREFIX ""
    FOLDER "GoodsackEngine"
    DESCRIPTION "GoodsackEngine API"
    OUTPUT_NAME "goodsack"
)

# Link the include directory to the main project
target_include_directories(GoodsackEngine_api PUBLIC "${SRC_DIR}")
target_link_libraries(GoodsackEngine_api GoodsackEngine_runtime)

#################################################################
# Example Projects
#################################################################
if(GOODSACK_EXAMPLES)
    set(DEMO_EXE_NAME "demo")
    set(DEMO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/demo/demo_hot")

    # Application icon (Windows only)
    if(MSVC)
        set(APP_ICON_RESOURCE "${DEMO_DIR}/appicon.rc")
    endif(MSVC)

    add_executable(${DEMO_EXE_NAME} "${DEMO_DIR}/main.cpp" "${DEMO_DIR}/demo_scenes.c" ${APP_ICON_RESOURCE})
    target_include_directories(${DEMO_EXE_NAME} PRIVATE ${DEMO_EXE_DIR})

    target_link_libraries(${DEMO_EXE_NAME} GoodsackEngine_runtime GoodsackEngine_api)
    target_compile_options(${DEMO_EXE_NAME} PUBLIC ${PROJECT_EXE_OPTIONS})

    target_compile_definitions(${DEMO_EXE_NAME} PUBLIC "_PROJ_DIR_ROOT=\"${DEMO_DIR}\"")
    target_compile_definitions(${DEMO_EXE_NAME} PUBLIC "_PROJ_DIR_DATA=\"${DEMO_DIR}/Resources\"")

    set_target_properties(${DEMO_EXE_NAME} PROPERTIES FOLDER "Projects")

    if(UNIX)
        set_target_properties(${DEMO_EXE_NAME}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        RESOURCES_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        )

        get_target_property(TESTRESPATH ${DEMO_EXE_NAME} RESOURCES_OUTPUT_DIRECTORY)
        message(STATUS "Resources Path specified at: ${TESTRESPATH}")

        # add_custom_command(TARGET ${DEMO_EXE_NAME} POST_BUILD
        #                    COMMAND ${CMAKE_COMMAND} -E copy_directory
        #                    ${CMAKE_SOURCE_DIR}/res/ $<TARGET_FILE_DIR:${DEMO_EXE_NAME}>/Resources)
        add_custom_command(TARGET ${DEMO_EXE_NAME} POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_directory
                           ${CMAKE_SOURCE_DIR}/res/ ${TESTRESPATH}/res)
    endif(UNIX)

endif()

#################################################################
# TestRunner Executable
#################################################################
if(GOODSACK_TEST)
    enable_testing()
    add_executable(GoodsackEngine_Test ${TEST_SOURCES} )

    ######################################################
    # Library: Google Test
    set(INSTALL_GTEST OFF CACHE INTERNAL "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)")
    find_package(GTest CONFIG QUIET)
    if(GTest_FOUND)
        message("GTest auto located")
    else()
        message("Failed to auto locate")
        set(GTEST_DIR "${LIB_DIR}/googletest") 
        add_subdirectory(${GTEST_DIR})
        target_include_directories(GoodsackEngine_Test PRIVATE 
            "${GTEST_DIR}" 
            "${GTEST_DIR}/googletest"
            "${GTEST_DIR}/googletest/include"
            "${GTEST_DIR}/googlemock"
            "${GTEST_DIR}/googlemock/include")
    endif()
    ######################################################

    #target_include_directories(GoodsackEngine_Test PRIVATE "${SRC_DIR}")
    target_link_libraries(GoodsackEngine_Test 
        GTest::gtest
        GTest::gtest_main
        GoodsackEngine_runtime
    )

    set_target_properties(GoodsackEngine_Test PROPERTIES FOLDER "GoodsackEngine/internal")
    target_compile_options(GoodsackEngine_Test PUBLIC ${TEST_EXE_OPTIONS})

    add_test(GoodsackEngine_Test GoodsackEngine_Test) # currently single-test model
    set_tests_properties(GoodsackEngine_Test PROPERTIES
        WORKING_DIRECTORY "${SRC_DIR}")

    if(UNIX)
        set_target_properties(GoodsackEngine_Test
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )
    endif(UNIX)
endif()

#################################################################
# Set configure_file
# NOTE: Should always be the last thing in our main CMakeLists.txt
#################################################################

configure_file(GoodsackEngineConfig.h.in GoodsackEngineConfig.h)