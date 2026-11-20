cmake_minimum_required(VERSION 3.20)
project(pokerandonx VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(PR_BUILD_CLI "Programa de línea de comandos (Linux)" ON)
option(PR_BUILD_APP "Interfaz Borealis" OFF)

add_subdirectory(core)
if(PR_BUILD_CLI AND NOT CMAKE_SYSTEM_NAME STREQUAL "NintendoSwitch")
    add_subdirectory(cli)
endif()
