option(BUILD_RUNTIME "Builds the runtime" ON)

if(BUILD_RUNTIME)
	message(STATUS "BUILD RUNTIME!")
    add_library(runtime runtime.cpp runtime.hpp)
else()
    	message(STATUS "DO NOT BUILD RUNTIME!")
endif()