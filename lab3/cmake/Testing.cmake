# GoogleTest + CTest, общие для всех проектов репозитория.
#
# Одна реализация на MSVC / MinGW / GCC / Clang: тесты регистрируются
# в CTest поштучно, поэтому `ctest` показывает каждый кейс отдельно,
# а движок выбирается один раз в корневом CMakeLists.

include_guard(GLOBAL)

include(FetchContent)
include(GoogleTest)

option(BUILD_TESTING "Собирать юнит-тесты" ON)

set(PPOIS_GTEST_VERSION "v1.17.0" CACHE STRING "Версия GoogleTest, подтягиваемая через FetchContent")

# Подключает GoogleTest (через FetchContent). Вызывается один раз на проект,
# после enable_testing() в корневом CMakeLists.
function(ppois_setup_googletest)
    if(TARGET GTest::gtest_main)
        return()
    endif()

    # Ключевой момент для Windows: gtest собирается с той же моделью CRT,
    # что и проект (/MD, а не /MT), иначе линковка тестов падает
    # с дублирующимися символами CRT.
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG ${PPOIS_GTEST_VERSION}
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(googletest)
endfunction()

# Объявляет тестовый бинарник на GoogleTest и регистрирует кейсы в CTest.
#
#   ppois_add_googletest(<target> [исходники...])
#
# Исходники можно не передавать, если они уже указаны в add_executable.
function(ppois_add_googletest target)
    if(NOT TARGET GTest::gtest_main)
        message(FATAL_ERROR "Сначала вызови ppois_setup_googletest()")
    endif()

    set(sources ${ARGN})
    if(sources)
        target_sources(${target} PRIVATE ${sources})
    endif()

    target_link_libraries(${target} PRIVATE GTest::gtest_main)

    # Служебный запуск тестов при сборке (gtest_discover_tests запускает
    # бинарник с --gtest_list_tests) тоже пишет default.profraw, если
    # включено профилирование. Он лежит в корне build-каталога, отчёты
    # его не подхватывают, а RunCoverage.cmake за собой подчищает.
    gtest_discover_tests(${target}
        DISCOVERY_TIMEOUT 120
        PROPERTIES TIMEOUT 120
    )
endfunction()
