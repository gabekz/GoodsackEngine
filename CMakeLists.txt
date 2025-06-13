#################################################################
# Goodsack Game Engine - CMake Configuration
# https://www.github.com/gabekz/GoodsackEngine


cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0077 OLD)

project(
    GoodsackEngine
    DESCRIPTION "Realtime 3D Rendering Engine"
    HOMEPAGE_URL "github.com/gabekz/GoodsackEngine")

file(READ "version.txt" ver)

string(REGEX MATCH "VERSION_MAJOR ([0-9]*)" _ ${ver})
set(GoodsackEngine_VERSION_MAJOR ${CMAKE_MATCH_1})

string(REGEX MATCH "VERSION_MINOR ([0-9]*)" _ ${ver})
set(GoodsackEngine_VERSION_MINOR ${CMAKE_MATCH_1})

string(REGEX MATCH "VERSION_PATCH ([0-9]*)" _ ${ver})
set(GoodsackEngine_VERSION_PATCH ${CMAKE_MATCH_1})

string(REGEX MATCH "VERSION_TWEAK ([0-9]*)" _ ${ver})
set(GoodsackEngine_VERSION_TWEAK ${CMAKE_MATCH_1})


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_COLOR_MAKEFILE ON)
include(colors)
define_colors()

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

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
# Source files + Paths
#################################################################
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/goodsack/gsk/gsk")
set(RES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/goodsack/gsk_data")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/goodsack/gsk/thirdparty")
set(API_DIR "${SRC_DIR}/api/llib")

if(UNIX)
set(_GSK_BUILD_OUT_DIR "${CMAKE_BINARY_DIR}")
else()
set(_GSK_BUILD_OUT_DIR "${CMAKE_BINARY_DIR}/output")
endif(UNIX)

set(GSK_ARCHIVE_OUT_DIR "${_GSK_BUILD_OUT_DIR}/lib_archive")
set(GSK_LIBRARY_OUT_DIR "${_GSK_BUILD_OUT_DIR}/lib")
set(GSK_RUNTIME_OUT_DIR "${_GSK_BUILD_OUT_DIR}/bin")
set(GSK_RESOURCES_OUT_DIR "${CMAKE_BINARY_DIR}")

set(GSK_GEN_OUT_DIR "${GSK_LIBRARY_OUT_DIR}/gsk_generated")

set(LIB_FOLDER_PATH "GoodsackEngine/ThirdParty")

set(TEST_SUBDIR_REGEX ".*__tests__.*")
# set(API_SUBDIR_REGEX ".*__api__.*")

# All Sources
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${SRC_DIR}/*.c"
    "${SRC_DIR}/*.cpp"
)

# REMOVE MAIN FILES
list(REMOVE_ITEM SOURCES
    "${SRC_DIR}/runtime/gsk_runtime_main.cpp"
    "${SRC_DIR}/prog/test/gsk_test_main.cpp"
    "${SRC_DIR}/prog/test/gsk_gen_main.cpp"
)

# Lua API-only sources
file(GLOB_RECURSE API_SOURCES CONFIGURE_DEPENDS
    "${API_DIR}/*.c"
    "${API_DIR}/*.cpp"
)

# Lua API-only sources
file(GLOB_RECURSE GSKGEN_SOURCES CONFIGURE_DEPENDS
    "${SRC_DIR}/prog/gsk_gen_main.cpp"
    "${SRC_DIR}/util/logger.c"
    "${SRC_DIR}/entity/component/ecs_component_layout.cpp"
    "${SRC_DIR}/entity/component/ecs_component_layout_loader.cpp"
)

# For each item in our list
foreach (item ${SOURCES})
    # Check item in SOURCES matches pattern TEST_SUBDIR_REGEX
    if ("${item}" MATCHES ${TEST_SUBDIR_REGEX})
        # current file from SOURCES to TEST_SOURCES
        list (REMOVE_ITEM SOURCES "${item}")
        list (APPEND TEST_SOURCES "${item}")
    endif ()
endforeach()

######## EXECUTABLES ############################################

option(GOODSACK_BUILD_EXAMPLES "Build example projects" ON)
option(GOODSACK_TEST "Build the TestRunner executable" ON)
option(GOODSACK_TEST_COVERAGE "Collect test coverage files" OFF)
option(GOODSACK_DEV "Development Mode" OFF)
option(GOODSACK_API_OMIT_PREFIX "Prefix" OFF)

option(GSKRT_USE_IO "Using the IO Runtime Library" ON)

######## subdirectories #########################################

add_subdirectory(goodsack/gsk/thirdparty)
add_subdirectory(goodsack/gsk/gsk)

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
    ARCHIVE_OUTPUT_DIRECTORY ${GSK_ARCHIVE_OUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${GSK_LIBRARY_OUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${GSK_RUNTIME_OUT_DIR}
    RESOURCES_OUTPUT_DIRECTORY ${GSK_RESOURCES_OUT_DIR}
)

target_include_directories(GoodsackEngine_runtime PUBLIC 
    ${SRC_DIR}
    ${PROJECT_BINARY_DIR}
    ${GSK_LIBRARY_OUT_DIR}
)

target_include_directories(GoodsackEngine_runtime PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/goodsack/gsk"
)

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


#################################################################
# GoodsackEngine Lua API Library
#################################################################
add_library(GoodsackEngine_api SHARED ${API_SOURCES})

set_target_properties(GoodsackEngine_api PROPERTIES
    PREFIX ""
    FOLDER "GoodsackEngine"
    DESCRIPTION "GoodsackEngine API"
    OUTPUT_NAME "goodsack"
    ARCHIVE_OUTPUT_DIRECTORY ${GSK_ARCHIVE_OUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${GSK_LIBRARY_OUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${GSK_RUNTIME_OUT_DIR}
    RESOURCES_OUTPUT_DIRECTORY ${GSK_RESOURCES_OUT_DIR}
)

# Link the include directory to the main project
target_include_directories(GoodsackEngine_api PUBLIC "${SRC_DIR}")
target_link_libraries(GoodsackEngine_api GoodsackEngine_runtime)

#################################################################
# Example Projects
#################################################################
if(GOODSACK_BUILD_EXAMPLES)
message("Building example projects..")
add_subdirectory(demo)
endif(GOODSACK_BUILD_EXAMPLES)

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
        message("GTest - auto located")
    else()
        message("GTest - Failed to auto locate")
        set(GTEST_DIR "${LIB_DIR}/googletest") 
        add_subdirectory(${GTEST_DIR})
        target_include_directories(GoodsackEngine_Test PRIVATE 
            "${GTEST_DIR}" 
            "${GTEST_DIR}/googletest"
            "${GTEST_DIR}/googletest/include"
            "${GTEST_DIR}/googlemock"
            "${GTEST_DIR}/googlemock/include")
    endif(GTest_FOUND)
    ######################################################

    #target_include_directories(GoodsackEngine_Test PRIVATE "${SRC_DIR}")
    target_link_libraries(GoodsackEngine_Test 
        GTest::gtest
        # GTest::gtest_main -- disabled gtest_main
        GoodsackEngine_runtime
    )
    # TODO: May need to rework this
    target_sources(GoodsackEngine_Test PRIVATE "${SRC_DIR}/prog/test/gsk_test_main.cpp")
    target_include_directories(GoodsackEngine_Test PRIVATE ${SRC_DIR} ${PROJECT_BINARY_DIR})

    set_target_properties(GoodsackEngine_Test PROPERTIES FOLDER "GoodsackEngine/internal")
    target_compile_options(GoodsackEngine_Test PUBLIC ${TEST_EXE_OPTIONS})

    add_test(GoodsackEngine_Test GoodsackEngine_Test) # currently single-test model
    set_tests_properties(GoodsackEngine_Test PROPERTIES
        WORKING_DIRECTORY "${SRC_DIR}")

    set_target_properties(GoodsackEngine_Test
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${GSK_ARCHIVE_OUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${GSK_LIBRARY_OUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${GSK_RUNTIME_OUT_DIR}
    RESOURCES_OUTPUT_DIRECTORY ${GSK_RESOURCES_OUT_DIR})

endif(GOODSACK_TEST)

#################################################################
# GSKGen Executable
#################################################################
    add_executable(GoodsackEngine_Gen ${GSKGEN_SOURCES} )

    #target_include_directories(GoodsackEngine_Test PRIVATE "${SRC_DIR}")
    target_link_libraries(GoodsackEngine_Gen
        ThirdParty_json
        ThirdParty_cglm
    )
    # TODO: May need to rework this
    target_sources(GoodsackEngine_Gen PRIVATE "${SRC_DIR}/prog/test/gsk_gen_main.cpp")
    target_include_directories(GoodsackEngine_Gen PRIVATE ${SRC_DIR} ${PROJECT_BINARY_DIR} ${GSK_LIBRARY_OUT_DIR})

    set_target_properties(GoodsackEngine_Gen PROPERTIES FOLDER "GoodsackEngine/internal")
    target_compile_options(GoodsackEngine_Gen PUBLIC ${GSKGEN_EXE_OPTIONS})

    set_target_properties(GoodsackEngine_Gen
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${GSK_ARCHIVE_OUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${GSK_LIBRARY_OUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${GSK_RUNTIME_OUT_DIR}
    RESOURCES_OUTPUT_DIRECTORY ${GSK_RESOURCES_OUT_DIR})

#################################################################
# Set configure_file
# NOTE: Should always be the last thing in our main CMakeLists.txt
#################################################################

configure_file(GoodsackEngineConfig.h.in "${GSK_GEN_OUT_DIR}/GoodsackEngineConfig.h")
