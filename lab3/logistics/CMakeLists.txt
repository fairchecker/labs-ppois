cmake_minimum_required(VERSION 3.21)

project(logistics
    VERSION 1.0.0
    DESCRIPTION "Transport and logistics management system (large-scale OOP)"
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake")
include(Common)
include(Coverage)
include(Testing)

enable_testing()

set(LOGISTICS_DOMAIN_SOURCES
    src/exceptions.cpp
    src/geo.cpp
    src/cargo.cpp
    src/clients.cpp
    src/orders.cpp
    src/tracking.cpp
    src/warehouse.cpp
    src/finance.cpp
    src/services.cpp
    src/vehicles.cpp
    src/people.cpp
    src/company.cpp
)

add_library(logistics_lib STATIC
    ${LOGISTICS_DOMAIN_SOURCES}
    src/cli/console_app.cpp
)
add_library(ppois::logistics ALIAS logistics_lib)

target_include_directories(logistics_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
ppois_set_cxx_standard(logistics_lib)
ppois_enable_warnings(logistics_lib)

add_executable(logistics src/main.cpp)
target_link_libraries(logistics PRIVATE ppois::logistics)
ppois_set_cxx_standard(logistics)
ppois_enable_warnings(logistics)

if(BUILD_TESTING)
    ppois_setup_googletest()

    add_executable(test_logistics
        tests/test_geo.cpp
        tests/test_cargo.cpp
        tests/test_vehicles.cpp
        tests/test_people.cpp
        tests/test_clients.cpp
        tests/test_orders.cpp
        tests/test_tracking.cpp
        tests/test_warehouse.cpp
        tests/test_finance.cpp
        tests/test_services.cpp
        tests/test_company.cpp
        tests/test_cli.cpp
    )
    target_link_libraries(test_logistics PRIVATE logistics_lib)
    ppois_set_cxx_standard(test_logistics)
    ppois_enable_warnings(test_logistics)
    ppois_add_googletest(test_logistics)

    ppois_enable_coverage(test_logistics logistics_lib
        ${LOGISTICS_DOMAIN_SOURCES}
        src/cli/console_app.cpp
    )
endif()
