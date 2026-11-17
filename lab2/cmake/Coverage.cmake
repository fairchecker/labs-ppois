# Покрытие кода через LLVM source-based coverage (llvm-profdata + llvm-cov).
#
# Почему так: один и тот же тулчейн clang + llvm-cov одинаково работает
# на Linux, macOS и Windows, тогда как gcov/lcov живут только на GCC
# и на Windows не собираются в принципе.
#
# Использование:
#   cmake -S <проект> -B build -DCODE_COVERAGE=ON -DCMAKE_CXX_COMPILER=clang++
#   cmake --build build --target coverage
# Отчёты: build/coverage/{report.txt, html/index.html, lcov.info, summary.json}

include_guard(GLOBAL)

option(CODE_COVERAGE "Собирать с покрытием кода (требует Clang)" OFF)
set(PPOIS_COVERAGE_MIN_LINES ""
    CACHE STRING "Минимальное покрытие по строкам в процентах (пусто — не проверять)")

# Подключает флаги инструментации к библиотеке и тестам и создаёт
# цель `coverage`, которая прогоняет ctest и строит отчёты.
#
#   ppois_enable_coverage(<test_target> <lib_target> [исходники...])
function(ppois_enable_coverage test_target lib_target)
    if(NOT CODE_COVERAGE)
        return()
    endif()

    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR
            "CODE_COVERAGE=ON требует Clang (сейчас ${CMAKE_CXX_COMPILER_ID}).\n"
            "Соберите с -DCMAKE_CXX_COMPILER=clang++ "
            "(CMakePresets/CI делают это автоматически).")
    endif()

    ppois_find_llvm_tools()

    # Профилирование и отображение покрытия — только для кода проекта:
    # gtest сюда не подключается и в отчёт не попадает.
    add_library(ppois_code_coverage INTERFACE)
    target_compile_options(ppois_code_coverage INTERFACE
        -O0
        -g
        -fno-inline
        -fprofile-instr-generate
        -fcoverage-mapping
    )
    target_link_options(ppois_code_coverage INTERFACE -fprofile-instr-generate)

    # Библиотека получает флаги компиляции, тестовый бинарник — и компиляции,
    # и линковки (рантайм профилирования нужен при линковке exe).
    target_link_libraries(${lib_target} PRIVATE ppois_code_coverage)
    target_link_libraries(${test_target} PRIVATE ppois_code_coverage)

    set(sources "")
    foreach(source IN LISTS ARGN)
        list(APPEND sources "${CMAKE_CURRENT_SOURCE_DIR}/${source}")
    endforeach()

    set(output_dir "${CMAKE_BINARY_DIR}/coverage")
    file(MAKE_DIRECTORY "${output_dir}")

    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND}
            "-DPP_COVERAGE_BUILD_DIR=${CMAKE_BINARY_DIR}"
            "-DPP_COVERAGE_CTEST=${CMAKE_CTEST_COMMAND}"
            "-DPP_COVERAGE_CONFIG=$<CONFIG>"
            "-DPP_COVERAGE_TEST_BINARY=$<TARGET_FILE:${test_target}>"
            "-DPP_COVERAGE_LLVM_PROFDATA=${PPOIS_LLVM_PROFDATA}"
            "-DPP_COVERAGE_LLVM_COV=${PPOIS_LLVM_COV}"
            "-DPP_COVERAGE_SOURCES=${sources}"
            "-DPP_COVERAGE_OUTPUT_DIR=${output_dir}"
            "-DPP_COVERAGE_MIN_LINES=${PPOIS_COVERAGE_MIN_LINES}"
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/RunCoverage.cmake"
        COMMENT "Тесты с покрытием + отчёты llvm-cov для ${test_target}"
        USES_TERMINAL
        VERBATIM
    )
endfunction()

# Ищет llvm-profdata и llvm-cov в типичных местах (в том числе
# C:\Program Files\LLVM\bin на GitHub Actions).
function(ppois_find_llvm_tools)
    if(PPOIS_LLVM_PROFDATA AND PPOIS_LLVM_COV)
        return()
    endif()

    file(GLOB llvm_versioned_hints "/usr/lib/llvm-*/bin")
    set(hints
        "$ENV{ProgramFiles}/LLVM/bin"
        "$ENV{ProgramW6432}/LLVM/bin"
        "${LLVM_TOOLS_BINARY_DIR}"
        /usr/bin
        /usr/local/bin
        /opt/homebrew/opt/llvm/bin
    )
    list(APPEND hints ${llvm_versioned_hints})

    find_program(PPOIS_LLVM_PROFDATA
        NAMES llvm-profdata llvm-profdata-21 llvm-profdata-20 llvm-profdata-19 llvm-profdata-18
        HINTS ${hints}
    )
    find_program(PPOIS_LLVM_COV
        NAMES llvm-cov llvm-cov-21 llvm-cov-20 llvm-cov-19 llvm-cov-18
        HINTS ${hints}
    )

    if(NOT PPOIS_LLVM_PROFDATA OR NOT PPOIS_LLVM_COV)
        message(FATAL_ERROR
            "CODE_COVERAGE=ON, но не найдены llvm-profdata и llvm-cov.\n"
            "Установите LLVM (Linux: apt install clang llvm, macOS: brew install llvm) "
            "или укажите -DPPOIS_LLVM_PROFDATA=... -DPPOIS_LLVM_COV=...")
    endif()

    message(STATUS "Coverage: ${PPOIS_LLVM_PROFDATA}, ${PPOIS_LLVM_COV}")
endfunction()
