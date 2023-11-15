set(DEMO_EXE_NAME "demo")
set(DEMO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/demo_hot")

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

set_target_properties(${DEMO_EXE_NAME} PROPERTIES FOLDER "GoodsackEngine/Projects")

#if(UNIX)
set_target_properties(${DEMO_EXE_NAME}
PROPERTIES
ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/output/lib/archive"
LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/output/lib"
RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/output/bin"
RESOURCES_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
if(UNIX)

    get_target_property(TESTRESPATH ${DEMO_EXE_NAME} RESOURCES_OUTPUT_DIRECTORY)
    message(STATUS "Resources Path specified at: ${TESTRESPATH}")

    # add_custom_command(TARGET ${DEMO_EXE_NAME} POST_BUILD
    #                    COMMAND ${CMAKE_COMMAND} -E copy_directory
    #                    ${CMAKE_SOURCE_DIR}/res/ $<TARGET_FILE_DIR:${DEMO_EXE_NAME}>/Resources)
    add_custom_command(TARGET ${DEMO_EXE_NAME} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/res/ ${TESTRESPATH}/res)
endif(UNIX)
