project(ImageFilterLib
        VERSION 1.0.0
        DESCRIPTION "High-performance JPEG image filter library"
        LANGUAGES CXX
)

add_library(${PROJECT_NAME} STATIC
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
        src/utils/FilterResult.cpp
        src/utils/ErrorHandlerChain.cpp
        src/utils/BorderHandler.cpp
        src/utils/LookupTables.cpp
        src/utils/CacheManager.cpp
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
)

find_package(stb REQUIRED)

# Общедоступные заголовки библиотеки
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ImageFilter
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${stb_SOURCE_DIR}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME ImageFilter
)

if(ENABLE_INSTALL)
    # Установка основной библиотеки
    install(TARGETS ${PROJECT_NAME}
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )

    # Установка заголовочных файлов
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.h"
    )

    # Конфигурационный файл пакета для CMake
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

# Подключаем тесты, если они включены
if(BUILD_TESTS)
    add_subdirectory(test)
endif()
