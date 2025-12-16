#[[
    Модуль генерации документации Doxygen для проекта ImageFilter.
    Обрабатывает флаг CMake-проекта BUILD_DOCS:
    - ищет установленный Doxygen;
    - настраивает выходную директорию для документации;
    - создаёт цель CMake `docs` для генерации документации;
    - выводит информативные сообщения и предупреждения.

    Предполагается, что модуль подключается из корневого CMakeLists.txt
    при установленном флаге BUILD_DOCS:

        if(BUILD_DOCS)
            include(cmake/Doxygen.cmake)
        endif()
]]

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


