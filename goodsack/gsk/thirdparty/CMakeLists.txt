######## 3rd-PARTY LIBRARIES ####################################

function(setup_thirdparty_lib LIBNAME)
    add_library(ThirdParty_${LIBNAME} INTERFACE)
    set_target_properties(ThirdParty_${LIBNAME} PROPERTIES FOLDER ${LIB_FOLDER_PATH}/${LIBNAME})
endfunction(set_library_folder)

add_library(ThirdParty INTERFACE)

#################################################################
# Library: STB
#################################################################
setup_thirdparty_lib(stb)
target_include_directories(ThirdParty_stb INTERFACE "${LIB_DIR}/stb")

#################################################################
# Library: JSON
#################################################################
setup_thirdparty_lib(json)

find_package(nlohmann_json QUIET) # NOTE: nlohmann-json in vcpkg
if(NOT nlohmann_json_FOUND)
    message(STATUS "(nlohmann_json) - failed to find package. Using local")
    set(JSON_BuildTests OFF CACHE INTERNAL "")
    set(JSON_INSTALL OFF CACHE INTERNAL "")
    add_subdirectory("${LIB_DIR}/json")
else()
    message(STATUS "(nlohmann_json) - auto located")
endif()
target_link_libraries(ThirdParty_json INTERFACE nlohmann_json::nlohmann_json)

#################################################################
# Library: cgltf
#################################################################
setup_thirdparty_lib(cgltf)
target_include_directories(ThirdParty_cgltf INTERFACE "${LIB_DIR}/cgltf")

#################################################################
# Library: CGLM
#################################################################
setup_thirdparty_lib(cglm)

set(CGLM_DIR "${LIB_DIR}/cglm")
add_subdirectory("${CGLM_DIR}" EXCLUDE_FROM_ALL)
set_target_properties(cglm PROPERTIES FOLDER ${LIB_FOLDER_PATH})

target_link_libraries(ThirdParty_cglm INTERFACE cglm cglm_headers) 
target_include_directories(ThirdParty_cglm INTERFACE cglm_headers)
target_compile_definitions(ThirdParty_cglm INTERFACE "CGLM_ALL_UNALIGNED")

#################################################################
# Library: GLFW
#################################################################
setup_thirdparty_lib(glfw)

set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")

find_package(glfw3 CONFIG QUIET)
if(glfw3_FOUND)
    target_link_libraries(ThirdParty_glfw INTERFACE glfw)
else()
    set(GLFW_DIR "${LIB_DIR}/glfw")
    add_subdirectory("${GLFW_DIR}")
    target_link_libraries(ThirdParty_glfw INTERFACE glfw ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})
    target_include_directories(ThirdParty_glfw INTERFACE "${GLFW_DIR}/include")
    target_compile_definitions(ThirdParty_glfw INTERFACE GLFW_INCLUDE_NONE)
endif()

#################################################################
# Library: GLAD
#################################################################
setup_thirdparty_lib(glad)

set(GLAD_DIR "${LIB_DIR}/glad")
add_library(glad STATIC "${GLAD_DIR}/src/gl.c")
set_target_properties(glad PROPERTIES FOLDER ${LIB_FOLDER_PATH})
if(UNIX)
    target_compile_options(glad PRIVATE "-fPIC")
endif(UNIX)
target_include_directories(glad PRIVATE "${GLAD_DIR}/include")

target_include_directories(ThirdParty_glad INTERFACE "${GLAD_DIR}/include")
target_link_libraries(ThirdParty_glad INTERFACE glad "${CMAKE_DL_LIBS}")

#################################################################
# Library: VulkanSDK
#################################################################
setup_thirdparty_lib(vulkan)

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
target_include_directories(ThirdParty_vulkan INTERFACE "${VULKAN_PATH}/Include")
#set(VULKAN_VERSION "1.0.33.0")
#set(VULKAN_LIB_LIST "vulkan")
# Link directory for vulkan-1
target_link_directories(ThirdParty_vulkan INTERFACE ${VULKAN_PATH}/Bin;${VULKAN_PATH}/Lib;)
target_link_libraries(ThirdParty_vulkan INTERFACE Vulkan::Vulkan)

#################################################################
# Library: ImGui
#################################################################
setup_thirdparty_lib(imgui)

find_package(imgui CONFIG QUIET)
if(imgui_FOUND)
    message(STATUS "(ImGui) Found package with auto-locate")
    target_link_libraries(ThirdParty_imgui INTERFACE imgui::imgui) 
else()
    set(IMGUI_DIR "${LIB_DIR}/imgui")
    message(STATUS "(ImGui) Using manual specified path: ${IMGUI_DIR}")

    add_library(imgui STATIC
                      "${IMGUI_DIR}/imgui.cpp"
                      "${IMGUI_DIR}/imgui_demo.cpp"
                      "${IMGUI_DIR}/imgui_draw.cpp"
                      "${IMGUI_DIR}/imgui_tables.cpp"
                      "${IMGUI_DIR}/imgui_widgets.cpp"

                      "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
                      "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
                      "${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp")
    if(UNIX)
        target_compile_options(imgui PRIVATE "-fPIC")
    endif(UNIX)

    target_include_directories(imgui PRIVATE ${IMGUI_DIR} "${IMGUI_DIR}/backends")
    target_link_libraries(imgui PRIVATE ThirdParty_glfw ThirdParty_vulkan "${CMAKE_DL_LIBS}")
    #target_include_directories(imgui PRIVATE "${VULKAN_PATH}/include")

    target_include_directories(ThirdParty_imgui INTERFACE ${IMGUI_DIR} "${IMGUI_DIR}/backends")
    target_link_libraries(ThirdParty_imgui INTERFACE imgui "${CMAKE_DL_LIBS}")
endif()

#################################################################
# Library: Lua
#################################################################
setup_thirdparty_lib(lua)

message(STATUS "(Lua) Attempting auto locate Lua using CMake.....")
find_package(Lua QUIET)
if(Lua_FOUND OR LUA_FOUND OR Lua51_FOUND OR LUA51_FOUND)
    message(STATUS "(Lua) Auto located.")
    #include_directories(/usr/local/include ${LUA_INCLUDE_DIR})
    target_link_libraries(ThirdParty_lua INTERFACE ${LUA_LIBRARIES})
    #target_link_libraries(${API_NAME} ${LUA_LIBRARIES})
    #target_include_directories(${RPOJECT_NAME} ${LUA_INCLUDE_DIR})
else()
    message(STATUS "(Lua) Failed to auto locate")
    set(LUA_DIR "${LIB_DIR}/lua-5.4.4")
    file(GLOB LUA_LIBRARIES "${LUA_DIR}/src/*.c" "${LUA_DIR}/src/*.h")
    add_library(lua ${LUA_LIBRARIES})
    if(UNIX)
        target_compile_options(lua PRIVATE "-fPIC")
    endif(UNIX)

    target_include_directories(ThirdParty_lua INTERFACE "${LUA_DIR}/src")
    target_link_libraries(ThirdParty_lua INTERFACE lua)

    #target_include_directories(${API_NAME} PUBLIC "${LUA_DIR}/src")
    #target_link_libraries(${API_NAME} lua)
endif()

#################################################################
# Library: OpenAL Soft
#################################################################
setup_thirdparty_lib(alsoft)

set(ALSOFT_INSTALL OFF CACHE INTERNAL "Install main library")
set(ALSOFT_EXAMPLES OFF CACHE INTERNAL "Build example programs")
set(ALSOFT_UTILS OFF CACHE INTERNAL "Build utility programs")
set(ALSOFT_NO_CONFIG_UTIL ON CACHE INTERNAL "Disable building the alsoft-config utility")

message(STATUS "(OpenAL) Auto-locating")
find_package(OpenAL CONFIG QUIET)
if(OpenAL_FOUND)
    message(STATUS "(OpenAL) successfully auto-located")
    target_link_libraries(ThirdParty_alsoft INTERFACE OpenAL::OpenAL)
else()
    message(STATUS "(OpenAL) failed to auto-locate")
    set(OPENAL_DIR "${LIB_DIR}/openal-soft") 
    add_subdirectory(${OPENAL_DIR})

    target_link_libraries(ThirdParty_alsoft INTERFACE OpenAL)
    target_include_directories(ThirdParty_alsoft INTERFACE 
        "${OPENAL_DIR}" 
        "${OPENAL_DIR}/include")
endif()