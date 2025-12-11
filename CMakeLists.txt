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
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    message(STATUS "Using Clang compiler: ${CMAKE_CXX_COMPILER}")
    # Включаем дополнительные предупреждения для Clang
    add_compile_options(-Wall -Wextra -Wpedantic)
else()
    message(WARNING "Clang compiler is preferred, but using: ${CMAKE_CXX_COMPILER_ID}")
endif()

# Оптимизация для Release сборки
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Опция для сборки тестов
option(BUILD_TESTS "Build unit tests" OFF)

# Опция для включения sanitizers
option(ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

# Опция для включения покрытия кода
option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)

# Опция для статического анализа
option(ENABLE_STATIC_ANALYSIS "Enable static code analysis" OFF)

# Настройка статического анализа
if(ENABLE_STATIC_ANALYSIS)
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
    
    # Проверяем наличие cppcheck (опционально)
    find_program(CPPCHECK_EXE
        NAMES cppcheck
        DOC "Path to cppcheck executable"
    )
    
    if(CPPCHECK_EXE)
        message(STATUS "cppcheck found at ${CPPCHECK_EXE} (use manually: cppcheck --project=compile_commands.json)")
    endif()
endif()

# Настройка sanitizers
if(ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-fsanitize=address,undefined)
        add_link_options(-fsanitize=address,undefined)
        message(STATUS "Sanitizers enabled: AddressSanitizer, UndefinedBehaviorSanitizer")
    else()
        message(WARNING "Sanitizers are only supported with Clang or GCC")
    endif()
endif()

# Настройка покрытия кода
if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        add_compile_options(--coverage)
        add_link_options(--coverage)
        message(STATUS "Code coverage enabled")
    else()
        message(WARNING "Code coverage is only supported with GCC or Clang")
    endif()
endif()

# Автоматическое скачивание STB через FetchContent
include(FetchContent)

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)

FetchContent_MakeAvailable(stb)

# Добавляем CLI11 для парсинга аргументов командной строки
FetchContent_Declare(
    CLI11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.4.1
)

FetchContent_MakeAvailable(CLI11)

# Добавляем Google Test если включены тесты
if(BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    
    FetchContent_MakeAvailable(googletest)
    
    # Отключаем установку Google Test
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
endif()

# Создаем объектную библиотеку с общими исходниками для переиспользования
add_library(ImageFilterLib OBJECT
    src/ImageProcessor.cpp
    src/utils/ParallelImageProcessor.cpp
    src/utils/ThreadPool.cpp
    src/filters/IFilter.cpp
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
    src/utils/Logger.cpp
    src/utils/FilterFactory.cpp
    src/utils/FilterResult.cpp
    src/utils/BorderHandler.cpp
    src/utils/BatchProcessor.cpp
    src/utils/LookupTables.cpp
    src/utils/Config.cpp
    src/utils/BMPHandler.cpp
    src/utils/PathValidator.cpp
)

# Подключаем заголовочные файлы к библиотеке
target_include_directories(ImageFilterLib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${stb_SOURCE_DIR}
    ${CLI11_SOURCE_DIR}/include
)

# Добавляем исполняемый файл
add_executable(ImageFilter
    src/main.cpp
    $<TARGET_OBJECTS:ImageFilterLib>
)

# Подключаем заголовочные файлы
target_include_directories(ImageFilter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${stb_SOURCE_DIR}
    ${CLI11_SOURCE_DIR}/include
)

# Устанавливаем выходную директорию
set_target_properties(ImageFilter PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Опция для установки
option(ENABLE_INSTALL "Enable installation" ON)

if(ENABLE_INSTALL)
    # Установка исполняемого файла
    install(TARGETS ImageFilter
        RUNTIME DESTINATION bin
    )
    
    # Установка заголовочных файлов
    install(DIRECTORY include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.h"
    )
    
    # Создаем конфигурационный файл для CMake
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)
    
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/ImageFilterConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )
    
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/ImageFilterConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ImageFilter
    )
endif()

# Создаем исполняемый файл для тестов
if(BUILD_TESTS)
    add_executable(ImageFilterTests
        tests/test_main.cpp
        tests/test_utils.cpp
        tests/test_ImageProcessor.cpp
        tests/test_filters.cpp
        tests/test_ParallelImageProcessor.cpp
        tests/test_edge_cases.cpp
        tests/test_integration.cpp
        $<TARGET_OBJECTS:ImageFilterLib>
    )
    
    target_include_directories(ImageFilterTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${stb_SOURCE_DIR}
        ${CLI11_SOURCE_DIR}/include
    )
    
    target_link_libraries(ImageFilterTests
        PRIVATE
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
    )
    
    # Включаем тесты в CTest
    include(CTest)
    enable_testing()
    add_test(NAME ImageFilterTests COMMAND ImageFilterTests)
    
    # Устанавливаем выходную директорию для тестов
    set_target_properties(ImageFilterTests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()

