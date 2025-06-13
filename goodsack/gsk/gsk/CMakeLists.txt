## SRC

#add_subdirectory(asset)
#
#add_subdirectory(components)
#
#add_subdirectory(core)
#
#add_subdirectory(ecs)
#
#add_subdirectory(model)
#
#add_subdirectory(runtime)
#
#add_subdirectory(tools)
#
#add_subdirectory(ui)
#
#add_subdirectory(util)
#
#add_subdirectory(wrapper)
message("USING IO = ${GSKRT_USE_IO}")
message(GSKRT_USE_IO)

if(GSKRT_USE_IO MATCHES ON)
    add_subdirectory(io)
endif()