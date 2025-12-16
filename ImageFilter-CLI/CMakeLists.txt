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

# Поиск и подключение CLI11 из vcpkg
find_package(CLI11 REQUIRED)

# Поиск и подключение nlohmann/json из vcpkg
find_package(nlohmann_json REQUIRED)

# Публичные include-директории для библиотеки
target_include_directories(ImageFilterCLI_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ImageFilterCLI_lib PUBLIC 
        ImageFilterLib
        CLI11::CLI11
        nlohmann_json
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
