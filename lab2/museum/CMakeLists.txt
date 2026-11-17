cmake_minimum_required(VERSION 3.21)

project(museum
    VERSION 1.0.0
    DESCRIPTION "Museum management system (large-scale OOP)"
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake")
include(Common)
include(Coverage)
include(Testing)

enable_testing()

add_library(museum_lib STATIC
    src/person.cpp
    src/roles.cpp
    src/visitor.cpp
    src/exhibits.cpp
    src/collection.cpp
    src/finance.cpp
    src/research_restoration.cpp
    src/security_facilities.cpp
    src/museum.cpp
    src/cli/console_app.cpp
)
add_library(ppois::museum ALIAS museum_lib)

target_include_directories(museum_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
ppois_set_cxx_standard(museum_lib)
ppois_enable_warnings(museum_lib)

add_executable(museum src/main.cpp)
target_link_libraries(museum PRIVATE ppois::museum)
ppois_set_cxx_standard(museum)
ppois_enable_warnings(museum)

if(BUILD_TESTING)
    ppois_setup_googletest()

    add_executable(test_museum
        tests/test_people.cpp
        tests/test_collection.cpp
        tests/test_finance.cpp
        tests/test_research.cpp
        tests/test_security.cpp
        tests/test_museum.cpp
        tests/test_cli.cpp
    )
    target_link_libraries(test_museum PRIVATE museum_lib)
    ppois_set_cxx_standard(test_museum)
    ppois_enable_warnings(test_museum)
    ppois_add_googletest(test_museum)

    ppois_enable_coverage(test_museum museum_lib
        src/person.cpp
        src/roles.cpp
        src/visitor.cpp
        src/exhibits.cpp
        src/collection.cpp
        src/finance.cpp
        src/research_restoration.cpp
        src/security_facilities.cpp
        src/museum.cpp
        src/cli/console_app.cpp
    )
endif()
