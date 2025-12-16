project(ImageFilterCLI)

# Библиотека с функционалом CLI (для переиспользования в юнит‑тестах и других целях)
add_library(ImageFilterCLI_lib STATIC
        cli/CommandParser.cpp
        cli/CommandExecutor.cpp
        cli/ImageProcessingHelper.cpp
        cli/FilterInfoDisplay.cpp
        cli/ProgressDisplay.cpp
        cli/FilterFactory.cpp
        cli/BatchProcessor.cpp
        preset/PresetManager.cpp
        preset/Config.cpp
        preset/ResumeStateManager.cpp
)

# Исполняемый файл CLI, использующий библиотеку
add_executable(${PROJECT_NAME}
        main.cpp
)

# Добавляем CLI11 для парсинга аргументов командной строки
find_package(CLI11)
# Добавляем nlohmann/json для работы с JSON
find_package(nlohmann-json)

# Публичные include-директории для библиотеки, чтобы их видели юнит‑тесты
target_include_directories(ImageFilterCLI_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${stb_SOURCE_DIR}
        ${CLI11_SOURCE_DIR}/include
        ${json_SOURCE_DIR}/include
)

# Библиотека CLI зависит от основной библиотеки фильтров
target_link_libraries(ImageFilterCLI_lib PUBLIC
        ImageFilterLib
)

# Исполняемый CLI зависит от своей библиотеки
target_link_libraries(${PROJECT_NAME} PRIVATE
        ImageFilterCLI_lib
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

if (ENABLE_INSTALL)
    install(TARGETS ${PROJECT_NAME}
            RUNTIME DESTINATION bin
    )
endif ()

# Подключаем тесты, если они включены
if(BUILD_TESTS)
    add_subdirectory(test)
endif()
