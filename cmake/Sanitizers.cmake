#[[
    Модуль настройки sanitizers для проекта ImageFilter.
    Обрабатывает флаг CMake-проекта ENABLE_SANITIZERS:
    - включает AddressSanitizer и UndefinedBehaviorSanitizer
      для поддерживаемых компиляторов (Clang, AppleClang, GCC);
    - выводит предупреждение для неподдерживаемых компиляторов.

    Предполагается, что модуль подключается из корневого CMakeLists.txt
    при установленном флаге ENABLE_SANITIZERS:

        if(ENABLE_SANITIZERS)
            include(cmake/Sanitizers.cmake)
        endif()
]]

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
        OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang"
        OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
    message(STATUS "Sanitizers enabled: AddressSanitizer, UndefinedBehaviorSanitizer")
else()
    message(WARNING "Sanitizers are only supported with Clang or GCC")
endif()


