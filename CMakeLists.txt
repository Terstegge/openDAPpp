cmake_minimum_required(VERSION 3.16)

# In multi-binary projects every target has to have a different
# name. So combine the target name with the current project name!
set(DAP_TARGET openDAP++_${PROJECT_NAME})

add_library(${DAP_TARGET} INTERFACE)

target_compile_options(${DAP_TARGET}
        INTERFACE -Ofast
)

target_sources(${DAP_TARGET} INTERFACE
        DAP_protocol.cpp
        DAP_log.cpp
)

target_include_directories(${DAP_TARGET}
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)
