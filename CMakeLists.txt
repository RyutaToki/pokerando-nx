cmake_minimum_required(VERSION 3.20)

option(PR_BUILD_CLI "Programa de línea de comandos (Linux)" ON)
option(PR_BUILD_APP "Interfaz Borealis" OFF)

if(PR_BUILD_APP)
    # Borealis elige plataforma (PLATFORM_DESKTOP / PLATFORM_SWITCH) y toolchain antes de project()
    set(BOREALIS_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/third_party/borealis/library)
    include(${BOREALIS_LIBRARY}/cmake/commonOption.cmake)
    include(${BOREALIS_LIBRARY}/cmake/toolchain.cmake)
endif()

project(pokerandonx VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(core)
if(PR_BUILD_CLI AND NOT CMAKE_SYSTEM_NAME STREQUAL "NintendoSwitch")
    add_subdirectory(cli)
endif()
if(PR_BUILD_APP)
    add_subdirectory(app)
endif()
