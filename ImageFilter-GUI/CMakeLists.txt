project(ImageFilterGUI
    VERSION 1.0.0
    DESCRIPTION "Qt5 Widgets GUI application for ImageFilter library"
    LANGUAGES CXX
)

# Явное указание стандарта C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Включаем автоматическую обработку Qt файлов
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Поиск Qt5 (Core, Widgets, Concurrent)
find_package(Qt5 REQUIRED COMPONENTS
    Core
    Widgets
    Concurrent
)

# Убеждаемся, что Qt5 найден
if(NOT Qt5_FOUND)
    message(FATAL_ERROR "Qt5 не найден. Убедитесь, что Qt5 установлен и настроен правильно.")
endif()

# Исполняемый файл приложения
add_executable(${PROJECT_NAME}
    main.cpp
    
    # Модели
    model/FilterChainModel.cpp
    model/FilterChainModel.h
    model/ImageModel.cpp
    model/ImageModel.h
    
    # Рабочие потоки
    worker/FilterAdapter.cpp
    worker/FilterAdapter.h
    worker/ImageProcessingWorker.cpp
    worker/ImageProcessingWorker.h
    
    # Контроллеры
    controller/ImageProcessorController.cpp
    controller/ImageProcessorController.h
    
    # GUI компоненты
    gui/BaseFilterConfigWidget.cpp
    gui/BaseFilterConfigWidget.h
    gui/FilterWidgetFactory.cpp
    gui/FilterWidgetFactory.h
    gui/filters/BrightnessFilterWidget.cpp
    gui/filters/BrightnessFilterWidget.h
    gui/filters/BrightnessFilterWidget.ui
    gui/filters/BlurFilterWidget.cpp
    gui/filters/BlurFilterWidget.h
    gui/filters/BlurFilterWidget.ui
    gui/filters/BoxBlurFilterWidget.cpp
    gui/filters/BoxBlurFilterWidget.h
    gui/filters/BoxBlurFilterWidget.ui
    gui/filters/ContrastFilterWidget.cpp
    gui/filters/ContrastFilterWidget.h
    gui/filters/ContrastFilterWidget.ui
    gui/filters/MotionBlurFilterWidget.cpp
    gui/filters/MotionBlurFilterWidget.h
    gui/filters/MotionBlurFilterWidget.ui
    gui/filters/SaturationFilterWidget.cpp
    gui/filters/SaturationFilterWidget.h
    gui/filters/SaturationFilterWidget.ui
    gui/MainWindow.cpp
    gui/MainWindow.h
    gui/MainWindow.ui
    gui/FilterChainPanel.cpp
    gui/FilterChainPanel.h
    gui/FilterChainPanel.ui
    gui/FilterItemWidget.ui
    gui/FilterItemWidget.cpp
    gui/FilterItemWidget.h
    gui/PreviewWidget.cpp
    gui/PreviewWidget.h
    gui/FilterSelectionDialog.cpp
    gui/FilterSelectionDialog.h
    gui/FilterSelectionDialog.ui
    gui/FilterProxyModel.cpp
    gui/FilterProxyModel.h
    gui/FilterListModel.cpp
    gui/FilterListModel.h

    # Ресурсы иконок
    gui/FilterItemWidget.qrc
)

# Настройка include-директорий
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Подключение библиотеки ImageFilter
# ImageFilterLib уже собран в родительском CMakeLists.txt
target_link_libraries(${PROJECT_NAME} PRIVATE
    ImageFilterLib
    Qt5::Core
    Qt5::Widgets
    Qt5::Concurrent
)

# Настройка выходной директории
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Установка приложения, если включена опция
if(ENABLE_INSTALL)
    install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
    )
endif()

