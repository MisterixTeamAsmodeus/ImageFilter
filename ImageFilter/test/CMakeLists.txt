project(ImageFilterLibTests)

# Исполняемый файл для тестов библиотеки
add_executable(${PROJECT_NAME}
    SafeMathTests.cpp
    ColorSpaceConverterTests.cpp
)

# Stb должен быть доступен через ImageFilterLib, но для тестов может понадобиться прямой доступ
# Получаем путь к Stb из родительского проекта
if(DEFINED STB_INCLUDE_DIR)
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${STB_INCLUDE_DIR}
    )
endif()

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ImageFilterLib
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

if(IMAGEFILTER_ENABLE_COVERAGE)
    # Применяем флаги покрытия к библиотеке и тестам через общий модуль.
    imagefilter_enable_coverage_for_target(${PROJECT_NAME})
    imagefilter_enable_coverage_for_target(ImageFilterLib)
endif()

