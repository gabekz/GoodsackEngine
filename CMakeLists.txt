#################################################################
# "Below" Game Engine - CMake Configuration
# https://www.github.com/gabekz/gl-renderer

cmake_minimum_required(VERSION 3.20)
project(
    GoodsackEngine
    VERSION 0.1.0.0
    DESCRIPTION "Realtime 3D Rendering Engine"
    HOMEPAGE_URL "github.com/gabekz/GoodsackEngine")

configure_file(GoodsackEngineConfig.h.in GoodsackEngineConfig.h)

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
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")
set(API_DIR "${SRC_DIR}/api")

set(TEST_SUBDIR_REGEX ".*__tests__.*")
# set(API_SUBDIR_REGEX ".*__api__.*")

# All Sources
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${SRC_DIR}/*.c"
    "${SRC_DIR}/*.cpp")

# Lua API-only sources
file(GLOB_RECURSE API_SOURCES CONFIGURE_DEPENDS
    "${API_DIR}/goodsack_engine.c"
    "${API_DIR}/time.c"
    #"${SRC_DIR}/core/device/device.c"
    "${API_DIR}/common.h")

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

#################################################################
# GoodsackEngine Runtime Library
#################################################################
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES
    PREFIX ""
    FOLDER "GoodsackEngine"
    DESCRIPTION "GoodsackEngine Runtime"
)

target_include_directories(${PROJECT_NAME} PUBLIC ${SRC_DIR} ${PROJECT_BINARY_DIR})

target_compile_options(${PROJECT_NAME} PUBLIC ${PROJECT_EXE_OPTIONS})

#################################################################
# GoodsackEngine Lua-API Library
#################################################################
set(API_NAME "GoodsackAPI")
add_library(${API_NAME} SHARED ${API_SOURCES})
set_target_properties(${API_NAME} PROPERTIES
    PREFIX ""
    FOLDER "GoodsackEngine"
    DESCRIPTION "GoodsackEngine API"
)

# Link the include directory to the main project
target_include_directories(${API_NAME} PUBLIC "${SRC_DIR}")
target_link_libraries(${API_NAME} ${PROJECT_NAME})

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

    target_link_libraries(${DEMO_EXE_NAME} ${PROJECT_NAME} ${API_NAME})
    target_compile_options(${DEMO_EXE_NAME} PUBLIC ${PROJECT_EXE_OPTIONS})

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
    set(TEST_EXE_NAME "testRunner")
    enable_testing()

    add_executable(${TEST_EXE_NAME} ${TEST_SOURCES} )
    target_include_directories(${TEST_EXE_NAME} PRIVATE "${SRC_DIR}")
    # Link test executable against gtest & gtest_main
    target_link_libraries(${TEST_EXE_NAME} 
        GTest::gtest 
        GTest::gtest_main
        ${PROJECT_NAME}
    )
    add_test(${TEST_EXE_NAME} ${TEST_EXE_NAME})
    set_tests_properties(${TEST_EXE_NAME} PROPERTIES
        WORKING_DIRECTORY "${SRC_DIR}")
    target_compile_options(${TEST_EXE_NAME} PUBLIC ${TEST_EXE_OPTIONS})

    if(UNIX)
        set_target_properties(${TEST_EXE_NAME}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
    endif(UNIX)
endif()

######## 3rd-PARTY LIBRARIES ####################################

#################################################################
# Library: STB
#################################################################
set(STB_DIR "${LIB_DIR}/stb")
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${STB_DIR})
target_link_libraries(${PROJECT_NAME} stb)

#################################################################
# Library: cgltf
#################################################################
set(CGLTF_DIR "${LIB_DIR}/cgltf")
add_library(cgltf INTERFACE)
target_include_directories(cgltf INTERFACE ${CGLTF_DIR})
target_link_libraries(${PROJECT_NAME} cgltf)

#################################################################
# Library: CGLM
#################################################################
set(CGLM_DIR "${LIB_DIR}/cglm")
add_subdirectory("${CGLM_DIR}" EXCLUDE_FROM_ALL)
target_link_libraries(${PROJECT_NAME} cglm cglm_headers) 
target_compile_definitions(${PROJECT_NAME} PUBLIC "CGLM_ALL_UNALIGNED")
target_compile_definitions(${DEMO_EXE_NAME} PUBLIC "CGLM_ALL_UNALIGNED")
target_compile_definitions(${TEST_EXE_NAME} PUBLIC "CGLM_ALL_UNALIGNED")

set_target_properties(cglm PROPERTIES FOLDER "ThirdParty")
#target_compile_definitions(${TEST_EXE_NAME} PUBLIC "CGLM_ALL_UNALIGNED")

#################################################################
# Library: GLFW
#################################################################
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")

find_package(glfw3 CONFIG QUIET)
if(glfw3_FOUND)
    target_link_libraries(${PROJECT_NAME} glfw)
else()
    set(GLFW_DIR "${LIB_DIR}/glfw")
    add_subdirectory("${GLFW_DIR}")
    target_link_libraries(${PROJECT_NAME} "glfw"
        "${GLFW_LIBRARIES}" "${CMAKE_DL_LIBS}")
    target_include_directories(${PROJECT_NAME} PRIVATE "${GLFW_DIR}/include")
    target_compile_definitions(${PROJECT_NAME} PRIVATE "GLFW_INCLUDE_NONE")
    target_link_libraries(${PROJECT_NAME} "glfw")
endif()


#################################################################
# Library: GLAD
#################################################################
set(GLAD_DIR "${LIB_DIR}/glad")
add_library("glad" STATIC "${GLAD_DIR}/src/gl.c")
set_target_properties(glad PROPERTIES FOLDER "ThirdParty")
if(UNIX)
    target_compile_options("glad" PRIVATE "-fPIC")
endif(UNIX)
target_include_directories("glad" PUBLIC "${GLAD_DIR}/include")
target_include_directories(${PROJECT_NAME} PUBLIC "${GLAD_DIR}/include")

target_link_libraries(${PROJECT_NAME} "glad" "${CMAKE_DL_LIBS}")

#################################################################
# Library: VulkanSDK
#################################################################
message(STATUS "(Vulkan) Attempting auto locate Vulkan using CMake.....")

# Find Vulkan Path using CMake's Vulkan Module
# This will return Boolean 'Vulkan_FOUND' indicating the status
# of find as success(ON) or fail(OFF).
# Include directory path - 'Vulkan_INCLUDE_DIRS' and 'Vulkan_LIBRARY'
# with required libraries.
find_package(Vulkan QUIET)

# Try extracting VulkanSDK path from ${Vulkan_INCLUDE_DIRS}
if (NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
	set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
	STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
endif()
if(NOT Vulkan_FOUND)
	# CMake may fail to locate the libraries but could be able to 
	# provide some path in Vulkan SDK include directory variable
	# 'Vulkan_INCLUDE_DIRS', try to extract path from this.
	message(STATUS "Failed to locate Vulkan SDK, retrying again...")
	if(EXISTS "${VULKAN_PATH}")
		message(STATUS "Successfully located the Vulkan SDK: ${VULKAN_PATH}")
	else()
		message("Error: Unable to locate Vulkan SDK.")
		# message("and specify manually path using 'VULKAN_SDK' and 'VULKAN_VERSION' variables in the CMakeLists.txt.")
		return()
	endif()
endif()
target_include_directories(${PROJECT_NAME} PUBLIC "${VULKAN_PATH}/Include")
#set(VULKAN_VERSION "1.0.33.0")
#set(VULKAN_LIB_LIST "vulkan")
# Link directory for vulkan-1
link_directories(${VULKAN_PATH}/Bin;${VULKAN_PATH}/Lib;)
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

#################################################################
# Library: ImGui
#################################################################
find_package(imgui CONFIG QUIET)
if(imgui_FOUND)
    target_link_libraries(${PROJECT_NAME} imgui::imgui) 
else()
    set(IMGUI_DIR "${LIB_DIR}/imgui")
    message(STATUS "(ImGui) Using manual specified path: ${IMGUI_DIR}")

    add_library("imgui" STATIC
                      "${IMGUI_DIR}/imgui.cpp"
                      "${IMGUI_DIR}/imgui_demo.cpp"
                      "${IMGUI_DIR}/imgui_draw.cpp"
                      "${IMGUI_DIR}/imgui_tables.cpp"
                      "${IMGUI_DIR}/imgui_widgets.cpp"

                      "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
                      "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
                      "${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp")
    if(UNIX)
        target_compile_options("imgui" PRIVATE "-fPIC")
    endif(UNIX)

    target_include_directories("imgui" PRIVATE ${IMGUI_DIR} "${IMGUI_DIR}/backends")
    target_link_libraries("imgui" "glfw" Vulkan::Vulkan "${CMAKE_DL_LIBS}")
    target_include_directories("imgui" PRIVATE "${VULKAN_PATH}/include")

    target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_DIR} "${IMGUI_DIR}/backends")
    target_link_libraries(${PROJECT_NAME} "imgui" "${CMAKE_DL_LIBS}")
endif()

#################################################################
# Library: Lua
#################################################################
message(STATUS "(Lua) Attempting auto locate Lua using CMake.....")
find_package(Lua QUIET)
if(Lua_FOUND OR LUA_FOUND OR Lua51_FOUND OR LUA51_FOUND)
    message(STATUS "(Lua) Auto located.")
    #include_directories(/usr/local/include ${LUA_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${LUA_LIBRARIES})
    target_link_libraries(${API_NAME} ${LUA_LIBRARIES})
    #target_include_directories(${RPOJECT_NAME} ${LUA_INCLUDE_DIR})
else()
    message(STATUS "(Lua) Failed to auto locate")
    set(LUA_DIR "${LIB_DIR}/lua-5.4.4")
    file(GLOB LUA_LIBRARIES "${LUA_DIR}/src/*.c" "${LUA_DIR}/src/*.h")
    #add_library("lua" ${LUA_LIBRARIES} "Actor")

    target_include_directories(${PROJECT_NAME} PUBLIC "${LUA_DIR}/src")
    target_link_libraries(${PROJECT_NAME} lua)

    target_include_directories(${API_NAME} PUBLIC "${LUA_DIR}/src")
    target_link_libraries(${API_NAME} lua)
endif()

#################################################################
# Library: JSON
#################################################################
find_package(nlohmann_json QUIET) # NOTE: nlohmann-json in vcpkg
if(NOT nlohmann_json_FOUND)
    message(STATUS "failed to find JSON")
    set(JSON_BuildTests OFF CACHE INTERNAL "")
    set(JSON_INSTALL OFF CACHE INTERNAL "")
    add_subdirectory("${LIB_DIR}/json")
else()
    message(STATUS "Auto-located JSON")
endif()
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)

#################################################################
# Library: Google Test
#################################################################
set(INSTALL_GTEST OFF CACHE INTERNAL "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)")
find_package(GTest CONFIG QUIET)
if(GTest_FOUND)
    message("GTest auto located")
else()
    message("Failed to auto locate")
    set(GTEST_DIR "${LIB_DIR}/googletest") 
    add_subdirectory(${GTEST_DIR})
    target_include_directories(${PROJECT_NAME} PRIVATE
        "${GTEST_DIR}" 
        "${GTEST_DIR}/googletest"
        "${GTEST_DIR}/googletest/include"
        "${GTEST_DIR}/googlemock"
        "${GTEST_DIR}/googlemock/include")
endif()

#################################################################
# Library: OpenAL Soft
#################################################################

set(ALSOFT_INSTALL OFF CACHE INTERNAL "Install main library")
set(ALSOFT_EXAMPLES OFF CACHE INTERNAL "Build example programs")
set(ALSOFT_UTILS OFF CACHE INTERNAL "Build utility programs")
set(ALSOFT_NO_CONFIG_UTIL ON CACHE INTERNAL "Disable building the alsoft-config utility")

message(STATUS "(OpenAL) Auto-locating")
find_package(OpenAL CONFIG QUIET)
if(OpenAL_FOUND)
    message(STATUS "(OpenAL) successfully auto-located")
    target_link_libraries(${PROJECT_NAME} OpenAL::OpenAL)
else()
    message(STATUS "(OpenAL) failed to auto-locate")
    set(OPENAL_DIR "${LIB_DIR}/openal-soft") 
    add_subdirectory(${OPENAL_DIR})

    target_link_libraries(${PROJECT_NAME} OpenAL)
    target_include_directories(${PROJECT_NAME} PRIVATE
        "${OPENAL_DIR}" 
        "${OPENAL_DIR}/include")
endif()
