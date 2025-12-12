cmake_minimum_required(VERSION 3.20)

project(ImageFilter 
    VERSION 1.0.0
    DESCRIPTION "High-performance JPEG image filter application"
    LANGUAGES CXX
)

# Определение платформы
if(WIN32)
    set(PLATFORM_NAME "Windows")
elseif(APPLE)
    set(PLATFORM_NAME "macOS")
elseif(UNIX)
    set(PLATFORM_NAME "Linux")
else()
    set(PLATFORM_NAME "Unknown")
endif()

message(STATUS "Building for platform: ${PLATFORM_NAME}")
message(STATUS "CMake generator: ${CMAKE_GENERATOR}")

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

# Оптимизация для Release сборки (кроссплатформенно)
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

# Опция для сборки тестов
option(BUILD_TESTS "Build unit tests" OFF)

# Опция для включения sanitizers
option(ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

# Опция для включения покрытия кода
option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)

# Опция для статического анализа
option(ENABLE_STATIC_ANALYSIS "Enable static code analysis" OFF)

# Опция для генерации документации Doxygen
option(BUILD_DOCS "Build Doxygen documentation" OFF)

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
    if(NOT BUILD_TESTS)
        message(WARNING "ENABLE_COVERAGE requires BUILD_TESTS to be ON. Enabling BUILD_TESTS automatically.")
        set(BUILD_TESTS ON CACHE BOOL "Build unit tests" FORCE)
    endif()
    
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        message(STATUS "Code coverage enabled")
    else()
        message(WARNING "Code coverage is only supported with GCC or Clang")
        set(ENABLE_COVERAGE OFF CACHE BOOL "Enable code coverage reporting" FORCE)
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
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(CLI11)

# Добавляем nlohmann/json для работы с JSON
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(json)

# Добавляем Google Test если включены тесты
if(BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
        GIT_SHALLOW TRUE
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
    src/utils/LoggerConfigurator.cpp
    src/utils/FilterFactory.cpp
    src/utils/FilterResult.cpp
    src/utils/ErrorHandlerChain.cpp
    src/utils/BorderHandler.cpp
    src/utils/BatchProcessor.cpp
    src/utils/LookupTables.cpp
    src/utils/CacheManager.cpp
    src/utils/Config.cpp
    src/utils/BMPHandler.cpp
    src/utils/PathValidator.cpp
    src/utils/BufferPool.cpp
    src/utils/FilterValidator.cpp
    src/utils/FilterValidationHelper.cpp
    src/utils/ImageValidator.cpp
    src/utils/ImageLoader.cpp
    src/utils/ImageSaver.cpp
    src/utils/ImageConverter.cpp
    src/utils/ColorSpaceConverter.cpp
    src/utils/FileSystemHelper.cpp
    src/utils/ResumeStateManager.cpp
    src/cli/CommandParser.cpp
    src/cli/CommandExecutor.cpp
    src/cli/ImageProcessingHelper.cpp
    src/cli/FilterInfoDisplay.cpp
    src/cli/ProgressDisplay.cpp
    src/cli/PresetManager.cpp
)

# Подключаем заголовочные файлы к библиотеке
target_include_directories(ImageFilterLib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${stb_SOURCE_DIR}
    ${CLI11_SOURCE_DIR}/include
    ${json_SOURCE_DIR}/include
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
    ${json_SOURCE_DIR}/include
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
        tests/test_BufferPool.cpp
        tests/test_Logger.cpp
        tests/test_LoggerConfigurator.cpp
        tests/test_Config.cpp
        tests/test_CommandParser.cpp
        tests/test_CommandExecutor.cpp
        tests/test_ImageLoader.cpp
        tests/test_ImageSaver.cpp
        tests/test_ImageConverter.cpp
        tests/test_FilterFactory.cpp
        tests/test_ThreadPool.cpp
        tests/test_ColorSpaceConverter.cpp
        tests/test_FilterResult.cpp
        tests/test_FilterValidationHelper.cpp
        tests/test_FilterValidator.cpp
        tests/test_ImageValidator.cpp
        tests/test_PathValidator.cpp
        tests/test_BMPHandler.cpp
        tests/test_LookupTables.cpp
        tests/test_ErrorHandlerChain.cpp
        tests/test_FileSystemHelper.cpp
        tests/test_CacheManager.cpp
        tests/test_ResumeStateManager.cpp
        tests/test_PresetManager.cpp
        tests/test_FilterInfoDisplay.cpp
        tests/test_ProgressDisplay.cpp
        tests/test_ImageProcessingHelper.cpp
        $<TARGET_OBJECTS:ImageFilterLib>
    )
    
    target_include_directories(ImageFilterTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${stb_SOURCE_DIR}
        ${CLI11_SOURCE_DIR}/include
        ${json_SOURCE_DIR}/include
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
    
    # Применяем флаги покрытия только к тестам
    if(ENABLE_COVERAGE)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(ImageFilterTests PRIVATE --coverage)
            target_link_options(ImageFilterTests PRIVATE --coverage)
            # Также применяем к библиотеке, так как тесты используют её объекты
            target_compile_options(ImageFilterLib PRIVATE --coverage)
            target_link_options(ImageFilterLib PRIVATE --coverage)
        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
            # Для Clang используем флаги совместимые с gcov/lcov
            target_compile_options(ImageFilterTests PRIVATE -fprofile-arcs -ftest-coverage)
            target_link_options(ImageFilterTests PRIVATE -fprofile-arcs -ftest-coverage)
            # Также применяем к библиотеке, так как тесты используют её объекты
            target_compile_options(ImageFilterLib PRIVATE -fprofile-arcs -ftest-coverage)
            target_link_options(ImageFilterLib PRIVATE -fprofile-arcs -ftest-coverage)
        endif()
    endif()
endif()

# Настройка генерации отчетов о покрытии
if(ENABLE_COVERAGE AND BUILD_TESTS)
    # Ищем инструменты для генерации отчетов
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # Для GCC используем lcov
        find_program(LCOV_EXECUTABLE
            NAMES lcov
            DOC "Path to lcov executable for code coverage"
        )
        find_program(GENHTML_EXECUTABLE
            NAMES genhtml
            DOC "Path to genhtml executable for HTML coverage reports"
        )
        
        if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
            set(COVERAGE_DIR ${CMAKE_BINARY_DIR}/coverage)
            set(COVERAGE_HTML_DIR ${COVERAGE_DIR}/html)
            
            add_custom_target(coverage
                COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_DIR}
                COMMAND ${LCOV_EXECUTABLE} --directory . --capture --output-file ${COVERAGE_DIR}/coverage.info --rc branch_coverage=1 --ignore-errors empty,inconsistent,format || true
                COMMAND ${LCOV_EXECUTABLE} --remove ${COVERAGE_DIR}/coverage.info '/usr/*' '*/_deps/*' '*/tests/*' --output-file ${COVERAGE_DIR}/coverage_filtered.info --rc branch_coverage=1 --ignore-errors empty,inconsistent,format,unused || true
                COMMAND ${GENHTML_EXECUTABLE} ${COVERAGE_DIR}/coverage_filtered.info --output-directory ${COVERAGE_HTML_DIR} --rc branch_coverage=1 --ignore-errors inconsistent,corrupt,category || ${CMAKE_COMMAND} -E echo "Warning: Could not generate HTML report. Check coverage.info file manually."
                COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated in ${COVERAGE_HTML_DIR}/index.html"
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                DEPENDS ImageFilterTests
                COMMENT "Generating code coverage report"
            )
            
            add_custom_target(coverage-clean
                COMMAND ${LCOV_EXECUTABLE} --directory . --zerocounters
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Cleaning coverage data"
            )
            
            message(STATUS "Coverage targets available: 'coverage' and 'coverage-clean'")
            message(STATUS "Run tests first, then 'cmake --build . --target coverage' to generate report")
        else()
            if(WIN32)
                message(STATUS "lcov/genhtml not found. Install with: choco install lcov or vcpkg install lcov")
            elseif(APPLE)
                message(STATUS "lcov/genhtml not found. Install with: brew install lcov")
            else()
                message(STATUS "lcov/genhtml not found. Install with: sudo apt-get install lcov (Debian/Ubuntu) or sudo yum install lcov (RHEL/CentOS)")
            endif()
        endif()
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        # Для Clang используем llvm-cov
        find_program(LLVM_COV_EXECUTABLE
            NAMES llvm-cov
            DOC "Path to llvm-cov executable for code coverage"
        )
        find_program(LLVM_PROFDATA_EXECUTABLE
            NAMES llvm-profdata
            DOC "Path to llvm-profdata executable for code coverage"
        )
        
        if(LLVM_COV_EXECUTABLE AND LLVM_PROFDATA_EXECUTABLE)
            set(COVERAGE_DIR ${CMAKE_BINARY_DIR}/coverage)
            set(COVERAGE_HTML_DIR ${COVERAGE_DIR}/html)
            
            add_custom_target(coverage
                COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_DIR}
                COMMAND ${CMAKE_COMMAND} -E echo "Note: For Clang coverage, run tests first, then use:"
                COMMAND ${CMAKE_COMMAND} -E echo "  ${LLVM_PROFDATA_EXECUTABLE} merge -sparse default.profraw -o ${COVERAGE_DIR}/coverage.profdata"
                COMMAND ${CMAKE_COMMAND} -E echo "  ${LLVM_COV_EXECUTABLE} show ${CMAKE_BINARY_DIR}/bin/ImageFilterTests -instr-profile=${COVERAGE_DIR}/coverage.profdata -format=html -output-dir=${COVERAGE_HTML_DIR}"
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Coverage setup for Clang. See messages above for manual commands."
            )
            
            message(STATUS "Coverage target 'coverage' available (manual commands required for Clang)")
        else()
            # Fallback: используем gcov для Clang (совместимость)
            find_program(LCOV_EXECUTABLE
                NAMES lcov
                DOC "Path to lcov executable for code coverage"
            )
            find_program(GENHTML_EXECUTABLE
                NAMES genhtml
                DOC "Path to genhtml executable for HTML coverage reports"
            )
            
            if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
                set(COVERAGE_DIR ${CMAKE_BINARY_DIR}/coverage)
                set(COVERAGE_HTML_DIR ${COVERAGE_DIR}/html)
                
                add_custom_target(coverage
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_DIR}
                    COMMAND ${LCOV_EXECUTABLE} --directory . --capture --output-file ${COVERAGE_DIR}/coverage.info --rc branch_coverage=1 --ignore-errors empty,inconsistent,format || true
                    COMMAND ${LCOV_EXECUTABLE} --remove ${COVERAGE_DIR}/coverage.info '/usr/*' '*/_deps/*' '*/tests/*' --output-file ${COVERAGE_DIR}/coverage_filtered.info --rc branch_coverage=1 --ignore-errors empty,inconsistent,format,unused || true
                    COMMAND ${GENHTML_EXECUTABLE} ${COVERAGE_DIR}/coverage_filtered.info --output-directory ${COVERAGE_HTML_DIR} --rc branch_coverage=1 --ignore-errors inconsistent,corrupt,category || ${CMAKE_COMMAND} -E echo "Warning: Could not generate HTML report. Check coverage.info file manually."
                    COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated in ${COVERAGE_HTML_DIR}/index.html"
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    DEPENDS ImageFilterTests
                    COMMENT "Generating code coverage report"
                )
                
                add_custom_target(coverage-clean
                    COMMAND ${LCOV_EXECUTABLE} --directory . --zerocounters
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Cleaning coverage data"
                )
                
                message(STATUS "Coverage targets available: 'coverage' and 'coverage-clean'")
                message(STATUS "Run tests first, then 'cmake --build . --target coverage' to generate report")
            else()
                if(WIN32)
                    message(STATUS "Coverage tools not found. Install lcov: choco install lcov or vcpkg install lcov")
                elseif(APPLE)
                    message(STATUS "Coverage tools not found. Install lcov: brew install lcov")
                else()
                    message(STATUS "Coverage tools not found. Install lcov: sudo apt-get install lcov (Debian/Ubuntu) or sudo yum install lcov (RHEL/CentOS)")
                endif()
            endif()
        endif()
    endif()
endif()

# Генерация документации Doxygen
if(BUILD_DOCS)
    # Ищем Doxygen
    find_program(DOXYGEN_EXECUTABLE
        NAMES doxygen
        DOC "Путь к исполняемому файлу Doxygen"
    )
    
    if(DOXYGEN_EXECUTABLE)
        # Настраиваем выходную директорию
        set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/docs/doxygen)
        
        # Создаем директорию для документации
        file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})
        
        # Добавляем цель для генерации документации
        add_custom_target(docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Генерация API документации с помощью Doxygen"
            VERBATIM
        )
        
        message(STATUS "Doxygen найден: ${DOXYGEN_EXECUTABLE}")
        message(STATUS "Документация будет сгенерирована в: ${DOXYGEN_OUTPUT_DIR}/html")
        message(STATUS "Для генерации документации выполните: cmake --build . --target docs")
    else()
        message(WARNING "Doxygen не найден. Установите Doxygen для генерации документации.")
        message(WARNING "На Ubuntu/Debian: sudo apt-get install doxygen")
        message(WARNING "На macOS: brew install doxygen")
        message(WARNING "На Windows: choco install doxygen.install")
    endif()
endif()

