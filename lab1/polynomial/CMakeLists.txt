cmake_minimum_required(VERSION 3.21)

project(polynomial
    VERSION 1.0.0
    DESCRIPTION "Polynomial class with arithmetic over real coefficients"
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake")
include(Common)
include(Coverage)
include(Testing)

enable_testing()

add_library(polynomial_lib STATIC src/polynomial.cpp)
add_library(ppois::polynomial ALIAS polynomial_lib)

target_include_directories(polynomial_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
ppois_set_cxx_standard(polynomial_lib)
ppois_enable_warnings(polynomial_lib)

add_executable(polynomial src/main.cpp)
target_link_libraries(polynomial PRIVATE ppois::polynomial)
ppois_set_cxx_standard(polynomial)
ppois_enable_warnings(polynomial)

if(BUILD_TESTING)
    ppois_setup_googletest()

    add_executable(test_polynomial tests/unit_tests.cpp tests/cli_tests.cpp)
    target_link_libraries(test_polynomial PRIVATE polynomial_lib)
    ppois_set_cxx_standard(test_polynomial)
    ppois_enable_warnings(test_polynomial)
    ppois_add_googletest(test_polynomial)

    ppois_enable_coverage(test_polynomial polynomial_lib
        src/polynomial.cpp
        src/main.cpp
    )
endif()
