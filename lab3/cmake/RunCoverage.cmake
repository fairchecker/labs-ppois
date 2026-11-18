# Скрипт сбора покрытия (запускается целью `coverage`).
#
# Прогоняет ctest с включённым профилированием LLVM, сливает .profraw
# в .profdata и строит отчёты: текстовый, HTML и lcov.info.
#
# Вызывается как:
#   cmake -DPP_COVERAGE_...=... -P RunCoverage.cmake

cmake_minimum_required(VERSION 3.21)

set(PP_COVERAGE_MIN_LINES "" CACHE STRING "")
set(PP_COVERAGE_CONFIG "Debug" CACHE STRING "")

foreach(required
        PP_COVERAGE_BUILD_DIR
        PP_COVERAGE_CTEST
        PP_COVERAGE_TEST_BINARY
        PP_COVERAGE_LLVM_PROFDATA
        PP_COVERAGE_LLVM_COV)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
        message(FATAL_ERROR "RunCoverage.cmake: не передан обязательный параметр -D${required}=<значение>")
    endif()
endforeach()

set(sources ${PP_COVERAGE_SOURCES})
if(NOT sources)
    message(FATAL_ERROR "RunCoverage.cmake: не переданы исходники (-DPP_COVERAGE_SOURCES=...)")
endif()

set(output_dir "${PP_COVERAGE_OUTPUT_DIR}")
set(raw_dir "${output_dir}/raw")
set(html_dir "${output_dir}/html")
set(profdata "${output_dir}/coverage.profdata")
# Тесты, санитайзеры и сам ctest пишут .profraw сюда; имя с %p не даёт
# параллельным процессам затереть друг друга (актуально для CTest -j).
set(ENV{LLVM_PROFILE_FILE} "${raw_dir}/coverage-%p.profraw")

file(REMOVE_RECURSE "${raw_dir}" "${html_dir}")
file(REMOVE "${PP_COVERAGE_BUILD_DIR}/default.profraw")
file(MAKE_DIRECTORY "${raw_dir}" "${output_dir}")

# 1. Тесты с профилированием.
execute_process(
    COMMAND "${PP_COVERAGE_CTEST}" --test-dir "${PP_COVERAGE_BUILD_DIR}"
            --output-on-failure -C "${PP_COVERAGE_CONFIG}"
    RESULT_VARIABLE ctest_result
)
if(NOT ctest_result EQUAL 0)
    message(FATAL_ERROR "Тесты упали (ctest: ${ctest_result}), покрытие не считается")
endif()

# 2. Слияние сырых профилей в единый .profdata.
file(GLOB profraw_files "${raw_dir}/*.profraw")
list(LENGTH profraw_files profraw_count)
if(profraw_count EQUAL 0)
    message(FATAL_ERROR
        "Не найдено ни одного .profraw в ${raw_dir}. "
        "Проверьте, что проект собран с -DCODE_COVERAGE=ON компилятором Clang.")
endif()

execute_process(
    COMMAND "${PP_COVERAGE_LLVM_PROFDATA}" merge -sparse ${profraw_files} -o "${profdata}"
    RESULT_VARIABLE merge_result
)
if(NOT merge_result EQUAL 0)
    message(FATAL_ERROR "llvm-profdata merge упал (код ${merge_result})")
endif()

set(ignore_regex "(_deps|/tests/|gmock|/gtest)")

# 3. Отчёты.
execute_process(
    COMMAND "${PP_COVERAGE_LLVM_COV}" report
            "${PP_COVERAGE_TEST_BINARY}" "-instr-profile=${profdata}"
            -ignore-filename-regex=${ignore_regex} ${sources}
    OUTPUT_FILE "${output_dir}/report.txt"
    RESULT_VARIABLE report_result
)
if(NOT report_result EQUAL 0)
    message(FATAL_ERROR "llvm-cov report упал (код ${report_result})")
endif()

execute_process(
    COMMAND "${PP_COVERAGE_LLVM_COV}" show -format=html -show-line-counts-or-regions
            "-output-dir=${html_dir}"
            "${PP_COVERAGE_TEST_BINARY}" "-instr-profile=${profdata}"
            -ignore-filename-regex=${ignore_regex} ${sources}
    RESULT_VARIABLE html_result
)
if(NOT html_result EQUAL 0)
    message(WARNING "llvm-cov show (HTML) упал (код ${html_result}), текстовый отчёт сохранён")
endif()

execute_process(
    COMMAND "${PP_COVERAGE_LLVM_COV}" export -format=lcov
            "${PP_COVERAGE_TEST_BINARY}" "-instr-profile=${profdata}"
            -ignore-filename-regex=${ignore_regex} ${sources}
    OUTPUT_VARIABLE lcov_data
    RESULT_VARIABLE lcov_result
)
if(lcov_result EQUAL 0)
    file(WRITE "${output_dir}/lcov.info" "${lcov_data}")
else()
    message(WARNING "llvm-cov export (lcov) упал (код ${lcov_result})")
endif()

# 4. Сводка в JSON + проверка минимального порога.
execute_process(
    COMMAND "${PP_COVERAGE_LLVM_COV}" export --summary-only -format=text
            "${PP_COVERAGE_TEST_BINARY}" "-instr-profile=${profdata}"
            -ignore-filename-regex=${ignore_regex} ${sources}
    OUTPUT_VARIABLE summary_json
    RESULT_VARIABLE summary_result
)
if(NOT summary_result EQUAL 0)
    message(FATAL_ERROR "llvm-cov export --summary-only упал (код ${summary_result})")
endif()
file(WRITE "${output_dir}/summary.json" "${summary_json}")

string(JSON lines_percent ERROR_VARIABLE json_error GET "${summary_json}" data 0 totals lines percent)
if(json_error)
    # Старые версии llvm-cov печатают сводку без обёртки "data".
    string(JSON lines_percent ERROR_VARIABLE json_error GET "${summary_json}" lines percent)
endif()
if(json_error)
    message(WARNING "Не удалось прочитать покрытие по строкам из summary.json: ${json_error}")
    set(lines_percent "")
else()
    string(JSON functions_percent GET "${summary_json}" data 0 totals functions percent)
    string(REGEX REPLACE "\\..*" "" lines_percent_short "${lines_percent}")
    string(REGEX REPLACE "\\..*" "" functions_percent_short "${functions_percent}")
    message(STATUS "Покрытие: строки ${lines_percent_short}%, функции ${functions_percent_short}%")
endif()

message(STATUS "Отчёты: ${output_dir} (report.txt, html/index.html, lcov.info, summary.json)")

if(PP_COVERAGE_MIN_LINES AND lines_percent AND lines_percent LESS PP_COVERAGE_MIN_LINES)
    message(FATAL_ERROR
        "Покрытие строк ${lines_percent}% ниже требуемого порога ${PP_COVERAGE_MIN_LINES}%")
endif()
