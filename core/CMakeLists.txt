file(GLOB_RECURSE PR_CORE_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE PR_DATA_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/data/*)

set(PR_EMBEDDED_CPP ${CMAKE_CURRENT_BINARY_DIR}/embedded_data.cpp)
add_custom_command(
    OUTPUT ${PR_EMBEDDED_CPP}
    COMMAND ${CMAKE_COMMAND} -DDATA_DIR=${CMAKE_CURRENT_SOURCE_DIR}/data -DOUTPUT=${PR_EMBEDDED_CPP}
            -P ${PROJECT_SOURCE_DIR}/cmake/embed_data.cmake
    DEPENDS ${PR_DATA_FILES} ${PROJECT_SOURCE_DIR}/cmake/embed_data.cmake
    COMMENT "Incrustando core/data")

add_library(pokerando_core STATIC ${PR_CORE_SOURCES} ${PR_EMBEDDED_CPP})
target_include_directories(pokerando_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(pokerando_core PRIVATE -Wall -Wextra)
