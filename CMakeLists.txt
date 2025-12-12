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

# Предпочтительный компилятор Clang
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Using Clang compiler: ${CMAKE_CXX_COMPILER}")
    # Включаем дополнительные предупреждения для Clang
    add_compile_options(-Wall -Wextra -Wpedantic)
else()
    message(WARNING "Clang compiler is preferred, but using: ${CMAKE_CXX_COMPILER_ID}")
endif()

# Оптимизация для Release сборки
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Автоматическое скачивание STB через FetchContent
include(FetchContent)

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)

FetchContent_MakeAvailable(stb)

# Добавляем исполняемый файл
add_executable(ImageFilter
    src/main.cpp
    src/ImageProcessor.cpp
    src/utils/ParallelImageProcessor.cpp
    src/filters/GrayscaleFilter.cpp
    src/filters/GaussianBlurFilter.cpp
    src/filters/SharpenFilter.cpp
    src/filters/SepiaFilter.cpp
    src/filters/InvertFilter.cpp
    src/filters/BrightnessFilter.cpp
    src/filters/ContrastFilter.cpp
    src/filters/SaturationFilter.cpp
    src/filters/FlipHorizontalFilter.cpp
    src/filters/FlipVerticalFilter.cpp
    src/filters/Rotate90Filter.cpp
    src/filters/EdgeDetectionFilter.cpp
    src/filters/EmbossFilter.cpp
    src/filters/OutlineFilter.cpp
    src/filters/MotionBlurFilter.cpp
    src/filters/MedianFilter.cpp
    src/filters/NoiseFilter.cpp
    src/filters/PosterizeFilter.cpp
    src/filters/ThresholdFilter.cpp
    src/filters/VignetteFilter.cpp
    src/filters/BoxBlurFilter.cpp
)

# Подключаем заголовочные файлы
target_include_directories(ImageFilter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${stb_SOURCE_DIR}
)

# Устанавливаем выходную директорию
set_target_properties(ImageFilter PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

