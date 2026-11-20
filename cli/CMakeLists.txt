add_executable(pokerando-cli main.cpp)
target_link_libraries(pokerando-cli PRIVATE pokerando_core)

add_executable(rng-hash-check ${PROJECT_SOURCE_DIR}/tests/cpp/rng_hash_check.cpp)
target_link_libraries(rng-hash-check PRIVATE pokerando_core)

add_executable(settings-check ${PROJECT_SOURCE_DIR}/tests/cpp/settings_check.cpp)
target_link_libraries(settings-check PRIVATE pokerando_core)

add_executable(nds-check ${PROJECT_SOURCE_DIR}/tests/cpp/nds_check.cpp)
target_link_libraries(nds-check PRIVATE pokerando_core)
