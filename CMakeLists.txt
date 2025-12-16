cmake_minimum_required(VERSION 3.20)

project(ImageFilter 
    VERSION 1.0.0
    DESCRIPTION "High-performance JPEG image filter application"
    LANGUAGES CXX
)

# Явное указание стандарта C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Подключаем модуль покрытия при активной опции
include(cmake/CodeCoverage.cmake)

# Опция для установки
option(ENABLE_INSTALL "Enable installation" ON)
# Опция для сборки консольного приложения
option(IMAGEFILTER_BUILD_CLI "Build console application" OFF)
# Опция для сборки Qt5 GUI-приложения
option(IMAGEFILTER_BUILD_GUI "Build Qt5 GUI application" OFF)
# Опция для сборки тестов
option(BUILD_TESTS "Build unit tests" OFF)
# Опция для включения флагов покрытия кода
option(IMAGEFILTER_ENABLE_COVERAGE "Enable code coverage flags for supported compilers" OFF)
# Опция для включения sanitizers
option(ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
# Опция для статического анализа
option(ENABLE_STATIC_ANALYSIS "Enable static code analysis" OFF)
# Опция для генерации документации Doxygen
option(BUILD_DOCS "Build Doxygen documentation" OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    message(STATUS "Using Clang compiler: ${CMAKE_CXX_COMPILER}")
    # Включаем дополнительные предупреждения для Clang
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Оптимизация для Release сборки (кроссплатформенно)
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

# Подключаем тесты, если они включены
if(BUILD_TESTS)
    enable_testing()
    include(CTest)

    include(FetchContent)

    # Отключаем установку Google Test
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK ON CACHE BOOL "" FORCE)

    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0
            GIT_SHALLOW TRUE
    )

    FetchContent_MakeAvailable(googletest)

    # Создаём цели покрытия, если модуль подключён и поддерживает их создание
    if(IMAGEFILTER_ENABLE_COVERAGE)
        if(COMMAND imagefilter_add_coverage_targets)
            imagefilter_add_coverage_targets()
        else()
            message(WARNING
                "IMAGEFILTER_ENABLE_COVERAGE включен, но функция imagefilter_add_coverage_targets "
                "недоступна. Убедитесь, что модуль CodeCoverage.cmake обновлён."
            )
        endif()
    endif()
endif()

# Подкаталог с библиотекой ядра
add_subdirectory(ImageFilter)

# Подкаталог с консольным приложением (по флагу)
if(IMAGEFILTER_BUILD_CLI)
    add_subdirectory(ImageFilter-CLI)
endif()

# Подкаталог с Qt5 GUI-приложением (по флагу)
if(IMAGEFILTER_BUILD_GUI)
    add_subdirectory(ImageFilter-GUI)
endif()

# Генерация документации Doxygen
if(BUILD_DOCS)
    include(cmake/Doxygen.cmake)
endif()

# Настройка sanitizers
if(ENABLE_SANITIZERS)
    include(cmake/Sanitizers.cmake)
endif()

# Настройка статического анализа
if(ENABLE_STATIC_ANALYSIS)
    include(cmake/StaticAnalysis.cmake)
endif()

