#[[
    Модуль настройки флагов покрытия кода для проекта ImageFilter.

    Назначение:
    - Централизованно определить флаги компиляции и линковки покрытия
      для разных компиляторов и ОС.
    - Предоставить функции для применения этих флагов к отдельным целям.

    Поддерживаемые конфигурации:
    - GCC (Linux/MinGW/WSL и т.п.): используется gcov (--coverage).
    - Clang / AppleClang (Linux/macOS): используется llvm-profdata/llvm-cov
      с флагами -fprofile-instr-generate -fcoverage-mapping.
    - MSVC / ClangCL (Windows, фронтенд MSVC):
      автоматическая настройка флагов не выполняется. Для таких конфигураций
      рекомендуется использовать встроенные инструменты покрытия Visual Studio
      или внешние утилиты (например, OpenCppCoverage).

    Использование:
    1) В корневом CMakeLists.txt включить модуль при активной опции:

         option(IMAGEFILTER_ENABLE_COVERAGE "Enable code coverage flags" OFF)
         if(IMAGEFILTER_ENABLE_COVERAGE)
             include(${CMAKE_CURRENT_SOURCE_DIR}/ImageFilter/cmake/CodeCoverage.cmake)
         endif()

    2) После создания целей (в соответствующих CMakeLists.txt) вызвать:

         if(IMAGEFILTER_ENABLE_COVERAGE)
             if(COMMAND imagefilter_enable_coverage_for_target)
                 imagefilter_enable_coverage_for_target(ImageFilterLib)
                 imagefilter_enable_coverage_for_target(ImageFilterLibTests)
             endif()
         endif()

    Флаги применяются только для конфигураций Debug и RelWithDebInfo.
]]

include_guard()

# Внутренняя функция: определяет флаги покрытия для текущего компилятора.
function(imagefilter_get_coverage_flags out_compile out_link)
    set(_compile_flags "")
    set(_link_flags "")

    # Определяем тип компилятора и фронтенд (важно для ClangCL)
    if(MSVC OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        # Для MSVC/ClangCL автоматическую настройку не выполняем,
        # так как поддержка /PROFILE и генерации отчётов сильно зависит
        # от установленного инструментария Visual Studio.
        message(WARNING
            "ImageFilter: автоматическая настройка флагов покрытия для MSVC/ClangCL "
            "не поддерживается. Используйте инструменты покрытия Visual Studio или "
            "внешние утилиты (например, OpenCppCoverage)."
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # GCC: классическая схема gcov/gcovr/lcov
        set(_compile_flags
            -O0
            -g
            --coverage
        )
        set(_link_flags
            --coverage
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
           OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        # Clang / AppleClang: профилирование через llvm-profdata / llvm-cov
        set(_compile_flags
            -O0
            -g
            -fprofile-instr-generate
            -fcoverage-mapping
        )
        set(_link_flags
            -fprofile-instr-generate
            -fcoverage-mapping
        )
    else()
        message(WARNING
            "ImageFilter: компилятор ${CMAKE_CXX_COMPILER_ID} не поддерживается "
            "для автоматической настройки покрытия кода."
        )
    endif()

    set(${out_compile} "${_compile_flags}" PARENT_SCOPE)
    set(${out_link} "${_link_flags}" PARENT_SCOPE)
endfunction()

# Публичная функция: включает флаги покрытия для указанной цели.
#
# Флаги применяются только для конфигураций Debug и RelWithDebInfo,
# чтобы не влиять на Release-сборки.
function(imagefilter_enable_coverage_for_target target)
    if(NOT TARGET "${target}")
        message(WARNING
            "ImageFilter: невозможно включить покрытие — цель '${target}' не найдена."
        )
        return()
    endif()

    # Не навязываем конкретный тип сборки, но предупреждаем,
    # если сборка не отладочная.
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        message(STATUS
            "ImageFilter: включено покрытие для цели '${target}' в конфигурации Release. "
            "Рекомендуется использовать Debug или RelWithDebInfo для более точного анализа."
        )
    endif()

    imagefilter_get_coverage_flags(_compile_flags _link_flags)

    if(_compile_flags STREQUAL "" AND _link_flags STREQUAL "")
        # Для данного компилятора флаги не настроены (например, MSVC)
        return()
    endif()

    # Применяем флаги только к нужным конфигурациям
    foreach(flag IN LISTS _compile_flags)
        target_compile_options(${target} PRIVATE
            "$<$<CONFIG:Debug,RelWithDebInfo>:${flag}>"
        )
    endforeach()

    foreach(flag IN LISTS _link_flags)
        target_link_options(${target} PRIVATE
            "$<$<CONFIG:Debug,RelWithDebInfo>:${flag}>"
        )
    endforeach()

    message(STATUS
        "ImageFilter: флаги покрытия кода применены к цели '${target}'."
    )
endfunction()

#[[
    Публичная функция: создаёт CMake-цели для генерации отчётов покрытия.

    Поддерживаемые бэкенды покрытия:
    - GCC (gcov/gcovr):
        - coverage       — запускает тесты (ctest) и, при наличии gcovr,
                           выводит сводный текстовый отчёт в консоль.
        - coverage_html  — формирует детализированный HTML‑отчёт (gcovr).
        - coverage_xml   — формирует XML‑отчёт в формате Cobertura (gcovr).
    - Clang / AppleClang (llvm-profdata / llvm-cov):
        - coverage       — запускает тесты (ctest) с генерацией .profraw
                           и, при наличии llvm-profdata/llvm-cov,
                           выводит сводный отчёт в консоль.
        - coverage_html  — формирует детализированный HTML‑отчёт (llvm-cov show),
                           если инструменты llvm-profdata и llvm-cov найдены.

    Особенности:
    - Цели создаются только при активных опциях BUILD_TESTS и IMAGEFILTER_ENABLE_COVERAGE.
    - Бэкенд выбирается автоматически по CMAKE_CXX_COMPILER_ID.
    - Отчёты помещаются в подкаталог coverage/<OS>-<COMPILER>/ внутри каталога сборки.
]]
function(imagefilter_add_coverage_targets)
    if(NOT BUILD_TESTS)
        message(WARNING
            "ImageFilter: попытка создать цели покрытия без включённых тестов "
            "(BUILD_TESTS=OFF). Цели coverage не будут созданы."
        )
        return()
    endif()

    if(NOT IMAGEFILTER_ENABLE_COVERAGE)
        message(WARNING
            "ImageFilter: imagefilter_add_coverage_targets вызвана при "
            "IMAGEFILTER_ENABLE_COVERAGE=OFF. Цели coverage не будут созданы."
        )
        return()
    endif()

    # Определяем платформенно‑зависимый подкаталог для отчётов
    string(TOLOWER "${CMAKE_SYSTEM_NAME}" _if_coverage_os)
    string(TOLOWER "${CMAKE_CXX_COMPILER_ID}" _if_coverage_compiler)
    set(_if_coverage_root_dir "${CMAKE_BINARY_DIR}/coverage")
    set(_if_coverage_output_dir
        "${_if_coverage_root_dir}/${_if_coverage_os}-${_if_coverage_compiler}"
    )

    # Определяем бэкенд покрытия в зависимости от компилятора.
    # ВАЖНО: для фронтенда MSVC (MSVC / ClangCL) мы не включаем
    # llvm‑backend, даже если CMAKE_CXX_COMPILER_ID == Clang,
    # т.к. флаги -fprofile-instr-generate / -fcoverage-mapping
    # не устанавливаются (см. imagefilter_get_coverage_flags),
    # а бинарники не содержат информации о покрытии для llvm-cov.
    set(_if_coverage_backend "none")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(_if_coverage_backend "gcovr")
    elseif((CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
            OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
           AND NOT (MSVC OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
        set(_if_coverage_backend "llvm")
    endif()

    if(_if_coverage_backend STREQUAL "none")
        message(WARNING
            "ImageFilter: текущий компилятор (${CMAKE_CXX_COMPILER_ID}) не поддерживается "
            "для автоматической генерации отчётов покрытия. Будет создана только цель "
            "'coverage', запускающая тесты."
        )
    endif()

    # Базовая цель: запуск всех тестов (без привязки к конкретному бэкенду).
    # Конкретные шаги для генерации отчётов добавляются ниже через add_custom_command.
    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E echo "ImageFilter: запуск тестов для сбора покрытия..."
        COMMENT "Запуск тестов (ctest) для измерения покрытия"
        USES_TERMINAL
    )
    
    if(_if_coverage_backend STREQUAL "gcovr")       # Ветка для GCC/gcovr
        imagefilter_add_coverage_targets_gcc("${_if_coverage_output_dir}")
    elseif(_if_coverage_backend STREQUAL "llvm")    # Ветка для Clang/AppleClang и llvm-profdata/llvm-cov
        imagefilter_add_coverage_targets_llvm("${_if_coverage_output_dir}")
    endif()

    message(STATUS
        "ImageFilter: цель 'coverage' настроена. Каталог отчётов: ${_if_coverage_output_dir}"
    )
endfunction()

#[[
    Внутренняя функция: настраивает цели покрытия для GCC/gcovr.

    Создаёт/дополняет цели:
    - coverage       — запускает ctest и, при наличии gcovr, выводит сводный текстовый отчёт.
    - coverage_html  — генерирует детализированный HTML‑отчёт (gcovr).
    - coverage_xml   — генерирует XML‑отчёт в формате Cobertura (gcovr).
]]
function(imagefilter_add_coverage_targets_gcc coverage_output_dir)
    # Пытаемся найти gcovr для генерации HTML/XML‑отчётов
    find_program(GCOVR_EXECUTABLE
        NAMES gcovr
        DOC "Путь к утилите gcovr для генерации отчётов покрытия"
    )

    if(NOT GCOVR_EXECUTABLE)
        message(WARNING
            "ImageFilter: утилита 'gcovr' не найдена. Цель 'coverage' будет "
            "только запускать тесты (ctest) без генерации HTML/XML отчётов."
        )
    endif()

    # Общие дополнительные аргументы для gcovr можно переопределить извне
    set(IMAGEFILTER_GCOVR_ADDITIONAL_ARGS "" CACHE STRING
        "Дополнительные аргументы командной строки для gcovr"
    )

    # coverage: запуск тестов и базовый сбор покрытия
    add_custom_command(
        TARGET coverage
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${coverage_output_dir}"
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        COMMENT "Запуск тестов (ctest) и сбор данных покрытия (GCC/gcovr)"
        VERBATIM
    )

    # Если доступен gcovr, создаём цели для HTML и XML‑отчётов
    if(GCOVR_EXECUTABLE)
        # Цель текстового отчёта (в консоль) после прогона тестов
        add_custom_command(
            TARGET coverage
            POST_BUILD
            COMMAND ${GCOVR_EXECUTABLE}
                --root "${CMAKE_SOURCE_DIR}"
                --filter "${CMAKE_SOURCE_DIR}/ImageFilter"
                --filter "${CMAKE_SOURCE_DIR}/ImageFilter-CLI"
                --exclude-directories "${CMAKE_BINARY_DIR}"
                ${IMAGEFILTER_GCOVR_ADDITIONAL_ARGS}
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            COMMENT "Генерация сводного отчёта покрытия (gcovr, текстовый формат)"
            VERBATIM
        )

        # Отдельная цель: XML‑отчёт (Cobertura)
        add_custom_target(coverage_xml
            COMMAND ${CMAKE_COMMAND} -E make_directory "${coverage_output_dir}"
            COMMAND ${GCOVR_EXECUTABLE}
                --root "${CMAKE_SOURCE_DIR}"
                --filter "${CMAKE_SOURCE_DIR}/ImageFilter"
                --filter "${CMAKE_SOURCE_DIR}/ImageFilter-CLI"
                --exclude-directories "${CMAKE_BINARY_DIR}"
                --xml
                --output "${coverage_output_dir}/coverage.xml"
                ${IMAGEFILTER_GCOVR_ADDITIONAL_ARGS}
            DEPENDS coverage
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            COMMENT "Генерация XML‑отчёта покрытия (gcovr, формат Cobertura)"
            USES_TERMINAL
            VERBATIM
        )

        # Отдельная цель: детализированный HTML‑отчёт
        add_custom_target(coverage_html
            COMMAND ${CMAKE_COMMAND} -E make_directory "${coverage_output_dir}/html"
            COMMAND ${GCOVR_EXECUTABLE}
                --root "${CMAKE_SOURCE_DIR}"
                --filter "${CMAKE_SOURCE_DIR}/ImageFilter"
                --filter "${CMAKE_SOURCE_DIR}/ImageFilter-CLI"
                --exclude-directories "${CMAKE_BINARY_DIR}"
                --html
                --html-details
                --output "${coverage_output_dir}/html/index.html"
                ${IMAGEFILTER_GCOVR_ADDITIONAL_ARGS}
            DEPENDS coverage
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            COMMENT "Генерация HTML‑отчёта покрытия (gcovr, детализированный отчёт)"
            USES_TERMINAL
            VERBATIM
        )
    endif()
endfunction()

#[[
    Внутренняя функция: настраивает цели покрытия для Clang/AppleClang и llvm-profdata/llvm-cov.

    Создаёт/дополняет цели:
    - coverage       — запускает ctest с установкой LLVM_PROFILE_FILE, агрегирует профили
                       через llvm-profdata и выводит сводный отчёт (llvm-cov report).
    - coverage_html  — формирует детализированный HTML‑отчёт (llvm-cov show).
]]
function(imagefilter_add_coverage_targets_llvm coverage_output_dir)
    # Путь к инструментам llvm-profdata и llvm-cov.
    # Сначала пытаемся найти их рядом с используемым компилятором Clang,
    # чтобы избежать несовместимости версий (например, между LLVM из VS
    # и отдельной установкой LLVM/Clang).
    get_filename_component(_if_llvm_clang_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)

    find_program(LLVM_PROFDATA_EXECUTABLE
        NAMES llvm-profdata
        HINTS "${_if_llvm_clang_dir}"
        DOC "Путь к утилите llvm-profdata для агрегации профилей покрытия"
    )
    find_program(LLVM_COV_EXECUTABLE
        NAMES llvm-cov
        HINTS "${_if_llvm_clang_dir}"
        DOC "Путь к утилите llvm-cov для генерации отчётов покрытия"
    )

    if(NOT LLVM_PROFDATA_EXECUTABLE OR NOT LLVM_COV_EXECUTABLE)
        message(WARNING
            "ImageFilter: утилиты 'llvm-profdata' и/или 'llvm-cov' не найдены. "
            "Цель 'coverage' будет только запускать тесты (ctest) без генерации отчётов."
        )
    endif()

    # Дополнительные аргументы для llvm-cov можно переопределить извне
    set(IMAGEFILTER_LLVM_COV_ADDITIONAL_ARGS "" CACHE STRING
        "Дополнительные аргументы командной строки для llvm-cov"
    )

    # Файл с профилем и агрегированный файл профиля для текущей сборки
    set(_if_llvm_profraw_file "${coverage_output_dir}/imagefilter_coverage.profraw")
    set(_if_llvm_profdata_file "${coverage_output_dir}/imagefilter_coverage.profdata")

    # Список исполняемых файлов тестов, для которых будет строиться отчёт
    set(_if_llvm_cov_binaries
        "${CMAKE_BINARY_DIR}/bin/ImageFilterLibTests${CMAKE_EXECUTABLE_SUFFIX}"
    )
    if(IMAGEFILTER_BUILD_CLI)
        list(APPEND _if_llvm_cov_binaries
            "${CMAKE_BINARY_DIR}/bin/ImageFilterCLITests${CMAKE_EXECUTABLE_SUFFIX}"
        )
    endif()

    # coverage: запуск тестов с установкой LLVM_PROFILE_FILE
    add_custom_command(
        TARGET coverage
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${coverage_output_dir}"
        COMMAND ${CMAKE_COMMAND} -E env
            "LLVM_PROFILE_FILE=${_if_llvm_profraw_file}"
            ${CMAKE_CTEST_COMMAND} --output-on-failure
        COMMENT "Запуск тестов (ctest) и сбор профилей покрытия (Clang/llvm)"
        VERBATIM
    )

    # Если инструменты llvm-profdata/llvm-cov доступны, добавляем генерацию текстового и HTML‑отчётов
    if(LLVM_PROFDATA_EXECUTABLE AND LLVM_COV_EXECUTABLE)
        # Текстовый отчёт в консоль после прогона тестов
        add_custom_command(
            TARGET coverage
            POST_BUILD
            COMMAND ${LLVM_PROFDATA_EXECUTABLE} merge -sparse
                "${_if_llvm_profraw_file}"
                -o "${_if_llvm_profdata_file}"
            COMMAND ${CMAKE_COMMAND} -E echo
                "ImageFilter: генерация сводного отчёта покрытия (llvm-cov report)..."
            COMMAND ${LLVM_COV_EXECUTABLE} report
                ${_if_llvm_cov_binaries}
                -instr-profile=${_if_llvm_profdata_file}
                ${IMAGEFILTER_LLVM_COV_ADDITIONAL_ARGS}
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            COMMENT "Генерация сводного отчёта покрытия (llvm-cov, текстовый формат)"
            VERBATIM
        )

        # Отдельная цель: детализированный HTML‑отчёт
        add_custom_target(coverage_html
            COMMAND ${CMAKE_COMMAND} -E make_directory "${coverage_output_dir}/html"
            COMMAND ${LLVM_COV_EXECUTABLE} show
                ${_if_llvm_cov_binaries}
                -instr-profile=${_if_llvm_profdata_file}
                -format=html
                -output-dir=${coverage_output_dir}/html
                -show-line-counts-or-regions
                ${IMAGEFILTER_LLVM_COV_ADDITIONAL_ARGS}
            DEPENDS coverage
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            COMMENT "Генерация HTML‑отчёта покрытия (llvm-cov, детализированный отчёт)"
            USES_TERMINAL
            VERBATIM
        )
    endif()
endfunction()

