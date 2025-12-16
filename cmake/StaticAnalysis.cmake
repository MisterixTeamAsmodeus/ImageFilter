#[[
    Модуль настройки статического анализа для проекта ImageFilter.
    Обрабатывает флаг CMake-проекта ENABLE_STATIC_ANALYSIS:
    - пытается найти clang-tidy;
    - при наличии clang-tidy подключает его как CMAKE_CXX_CLANG_TIDY;
    - при отсутствии clang-tidy усиливает предупреждения компилятора.

    Предполагается, что модуль подключается из корневого CMakeLists.txt
    при установленном флаге ENABLE_STATIC_ANALYSIS:

        if(ENABLE_STATIC_ANALYSIS)
            include(cmake/StaticAnalysis.cmake)
        endif()
]]

# Проверяем наличие clang-tidy
find_program(CLANG_TIDY_EXE
    NAMES clang-tidy
    DOC "Path to clang-tidy executable"
)

if(CLANG_TIDY_EXE)
    # Используем clang-tidy для статического анализа во время компиляции
    set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXE})
    message(STATUS "Static analysis enabled: clang-tidy found at ${CLANG_TIDY_EXE}")

    # Добавляем дополнительные флаги для более строгого анализа
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        add_compile_options(-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic)
    endif()
else()
    message(STATUS "clang-tidy not found, using compiler warnings for static analysis")

    # Используем дополнительные флаги компилятора для более строгого анализа
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        add_compile_options(-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic)
        message(STATUS "Static analysis enabled: using strict compiler warnings")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-Wpedantic -Wextra -Wall -Wformat=2 -Wstrict-aliasing=2)
        message(STATUS "Static analysis enabled: using strict compiler warnings")
    else()
        message(WARNING "Static analysis is limited for ${CMAKE_CXX_COMPILER_ID}")
    endif()
endif()


