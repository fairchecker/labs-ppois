cmake_minimum_required(VERSION 3.21)

project(englorus
    VERSION 1.0.0
    DESCRIPTION "English-Russian dictionary (BST)"
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake")
include(Common)
include(Coverage)
include(Testing)

enable_testing()

add_library(englorus_lib STATIC
    src/dictionary.cpp
    src/dictionary_node.cpp
)
add_library(ppois::englorus ALIAS englorus_lib)

target_include_directories(englorus_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
ppois_set_cxx_standard(englorus_lib)
ppois_enable_warnings(englorus_lib)

add_executable(englorus src/main.cpp)
target_link_libraries(englorus PRIVATE ppois::englorus)
ppois_set_cxx_standard(englorus)
ppois_enable_warnings(englorus)

if(BUILD_TESTING)
    ppois_setup_googletest()

    add_executable(test_dictionary tests/unit_tests.cpp tests/cli_tests.cpp)
    target_link_libraries(test_dictionary PRIVATE englorus_lib)
    ppois_set_cxx_standard(test_dictionary)
    ppois_enable_warnings(test_dictionary)
    ppois_add_googletest(test_dictionary)

    ppois_enable_coverage(test_dictionary englorus_lib
        src/dictionary.cpp
        src/dictionary_node.cpp
        src/main.cpp
    )
endif()
