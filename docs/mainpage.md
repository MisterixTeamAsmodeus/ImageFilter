# ImageFilter API Документация

Добро пожаловать в документацию API библиотеки ImageFilter - высокопроизводительного приложения для обработки изображений на C++20.

## Обзор

ImageFilter предоставляет набор классов и утилит для:
- Загрузки и сохранения изображений в форматах JPEG, PNG и BMP
- Применения различных фильтров обработки изображений
- Параллельной обработки больших изображений
- Создания пользовательских фильтров

## Основные компоненты

### ImageProcessor
Основной класс для работы с изображениями. Предоставляет методы для:
- Загрузки изображений из файлов
- Сохранения изображений в файлы
- Получения информации о размере и формате изображения
- Работы с альфа-каналом (RGBA)

**Пример использования:**
```cpp
ImageProcessor processor;
processor.loadFromFile("input.jpg");
processor.saveToFile("output.jpg");
```

### IFilter и FilterFactory
Интерфейс для всех фильтров обработки изображений и фабрика для их создания.

**Пример использования:**
```cpp
FilterFactory::getInstance().registerAll();
auto filter = FilterFactory::getInstance().create("grayscale", app);
filter->apply(processor);
```

### ParallelImageProcessor
Утилита для распараллеливания обработки изображений на несколько потоков.

**Пример использования:**
```cpp
ParallelImageProcessor::processRowsParallel(
    height, width,
    [&](int start_row, int end_row) {
        // Обработка диапазона строк
    }
);
```

## Примеры кода

В директории `examples/` доступны следующие примеры:

1. **example_basic_usage.cpp** - Базовое использование ImageProcessor
2. **example_filter_usage.cpp** - Применение фильтров через FilterFactory
3. **example_parallel_processing.cpp** - Параллельная обработка изображений
4. **example_custom_filter.cpp** - Создание пользовательского фильтра

## Быстрый старт

1. Загрузите изображение:
```cpp
ImageProcessor processor;
processor.loadFromFile("input.jpg");
```

2. Примените фильтр:
```cpp
FilterFactory::getInstance().registerAll();
auto filter = FilterFactory::getInstance().create("grayscale", app);
filter->apply(processor);
```

3. Сохраните результат:
```cpp
processor.saveToFile("output.jpg");
```

## Дополнительные ресурсы

- [Руководство пользователя](USER_GUIDE.md) - Подробное описание использования приложения
- [Руководство разработчика](DEVELOPER_GUIDE.md) - Информация для разработчиков
- [Примеры кода](../examples/README.md) - Примеры использования API

## Лицензия

Учебный проект для изучения обработки изображений на C++20.

