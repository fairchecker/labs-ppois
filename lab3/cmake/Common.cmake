# Общие настройки компиляции для всех проектов репозитория.
#
# Подключается одним вызовом из корневого CMakeLists проекта:
#   list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")
#   include(Common)

include_guard(GLOBAL)

# Единый стандарт C++ для проектов и тестов.
function(ppois_set_cxx_standard target)
    target_compile_features(${target} PUBLIC cxx_std_17)
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
endfunction()

# Предупреждения компилятора, одинаковые на всех платформах.
# MSVC получает /utf-8, иначе исходники с кириллицей дают C4819,
# а в некоторых конфигурациях — ошибку линковки.
function(ppois_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8 /EHsc)
    else()
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wnon-virtual-dtor
        )
    endif()
endfunction()
