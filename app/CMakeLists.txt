# App POKERM NX (Borealis). Plataforma: -DPLATFORM_DESKTOP=ON o -DPLATFORM_SWITCH=ON
set(BOREALIS_PATH ${PROJECT_SOURCE_DIR}/third_party/borealis)

# recursos: los de Borealis que hacen falta + los nuestros (fuentes, textos)
set(PR_RES_DIR ${CMAKE_BINARY_DIR}/resources)
set(BRLS_RESOURCES_DIR ${CMAKE_BINARY_DIR})
add_subdirectory(${BOREALIS_PATH}/library ${CMAKE_BINARY_DIR}/borealis)

file(GLOB_RECURSE PR_APP_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
if(PLATFORM_SWITCH)
    # userAppInit/userAppExit de Borealis: romfs, fuentes del sistema, idioma, red... Sin esto la app se cierra al arrancar.
    list(APPEND PR_APP_SOURCES ${BOREALIS_PATH}/library/lib/platforms/switch/switch_wrapper.c)
endif()
add_executable(pokerandonx ${PR_APP_SOURCES})
target_include_directories(pokerandonx PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pokerandonx PRIVATE pokerando_core borealis)
target_compile_options(pokerandonx PRIVATE -Wall -ffunction-sections -fdata-sections)
target_compile_definitions(pokerandonx PRIVATE PR_VERSION="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

add_custom_target(pr_resources ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PR_RES_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${BOREALIS_PATH}/resources/material ${PR_RES_DIR}/material
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${BOREALIS_PATH}/resources/font ${PR_RES_DIR}/font
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/romfs ${PR_RES_DIR}
    COMMENT "Copiando recursos")
add_dependencies(pokerandonx pr_resources)

if(PLATFORM_SWITCH)
    set(PR_ICON ${CMAKE_CURRENT_SOURCE_DIR}/icon.jpg)
    add_custom_target(pokermnx.nro ALL DEPENDS pokerandonx pr_resources
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${PR_RES_DIR}/font
        COMMAND ${NX_NACPTOOL_EXE} --create "POKERM NX" "RyutaToki" "${PROJECT_VERSION}" pokermnx.nacp
        COMMAND ${NX_ELF2NRO_EXE} pokerandonx.elf pokermnx.nro --icon=${PR_ICON} --nacp=pokermnx.nacp --romfsdir=${PR_RES_DIR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()
