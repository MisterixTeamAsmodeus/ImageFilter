# Руководство разработчика ImageFilter

## Содержание

1. [Архитектура проекта](#архитектура-проекта)
2. [Добавление новых фильтров](#добавление-новых-фильтров)
3. [Стиль кода](#стиль-кода)
4. [Процесс разработки](#процесс-разработки)
5. [Тестирование](#тестирование)

## Архитектура проекта

### Структура проекта

```
ImageFilter/
├── include/              # Заголовочные файлы
│   ├── filters/         # Интерфейсы и заголовки фильтров
│   ├── utils/           # Утилиты (параллельная обработка, пулы)
│   └── ImageProcessor.h # Основной класс для работы с изображениями
├── src/                 # Исходные файлы
│   ├── filters/         # Реализации фильтров
│   ├── utils/           # Реализации утилит
│   ├── ImageProcessor.cpp
│   └── main.cpp         # Точка входа приложения
├── tests/               # Тесты
└── benchmark/           # Инструменты для бенчмарка
```

### Основные компоненты

#### ImageProcessor

Класс для работы с изображениями. Предоставляет:
- Загрузку и сохранение изображений (JPEG, PNG, BMP)
- Управление данными изображения
- Преобразование между RGB и RGBA

#### IFilter

Базовый интерфейс для всех фильтров. Каждый фильтр должен:
- Реализовать метод `apply()` для применения фильтра
- Предоставить информацию о себе (имя, описание, категория)
- Указать требования к памяти и возможность in-place обработки

#### FilterFactory

Фабрика для создания фильтров по имени. Использует паттерн Factory для регистрации и создания фильтров.

#### ParallelImageProcessor

Утилита для параллельной обработки изображений. Разделяет изображение на полосы и обрабатывает их в параллельных потоках.

#### ThreadPool

Пул для переиспользования потоков. Поддерживает Dependency Injection через интерфейс IThreadPool.

## Добавление новых фильтров

### Шаг 1: Создание класса фильтра

Создайте новый класс, наследующийся от `IFilter`:

```cpp
// include/filters/MyFilter.h
#pragma once

#include <filters/IFilter.h>

class MyFilter : public IFilter
{
public:
    FilterResult apply(ImageProcessor& image) override;
    std::string getName() const override;
    std::string getDescription() const override;
    std::string getCategory() const override;
};
```

### Шаг 2: Реализация фильтра

```cpp
// src/filters/MyFilter.cpp
#include <filters/MyFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>

FilterResult MyFilter::apply(ImageProcessor& image)
{
    if (!image.isValid())
    {
        return FilterResult::failure(FilterError::InvalidImage, "Изображение не загружено");
    }

    // Валидация
    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();

    if (width <= 0 || height <= 0)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        return FilterResult::failure(FilterError::InvalidSize,
                                     "Размер изображения должен быть больше нуля", ctx);
    }

    // Применение фильтра
    auto* data = image.getData();
    
    ParallelImageProcessor::processRowsParallel(
        height,
        width,
        [width, channels, data](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    // Обработка пикселя
                    const auto pixel_offset = (static_cast<size_t>(y) * width + x) * channels;
                    // ... ваша логика обработки ...
                }
            }
        }
    );

    return FilterResult::success();
}

std::string MyFilter::getName() const
{
    return "my_filter";
}

std::string MyFilter::getDescription() const
{
    return "Описание моего фильтра";
}

std::string MyFilter::getCategory() const
{
    return "Цветовой"; // или "Геометрический", "Края и детали", и т.д.
}
```

### Шаг 3: Регистрация фильтра

Добавьте регистрацию в `FilterFactory::registerAll()`:

```cpp
// src/utils/FilterFactory.cpp
#include <filters/MyFilter.h>

void FilterFactory::registerAll()
{
    // ... существующие фильтры ...
    
    registerFilter("my_filter", [](const CLI::App& app) {
        // Извлечение параметров из CLI::App, если нужно
        return std::make_unique<MyFilter>();
    });
}
```

### Шаг 4: Добавление в CMakeLists.txt

Добавьте файлы фильтра в `CMakeLists.txt`:

```cmake
add_executable(ImageFilter
    # ... существующие файлы ...
    src/filters/MyFilter.cpp
)
```

### Шаг 5: Написание тестов

Создайте тесты для нового фильтра:

```cpp
// tests/test_filters.cpp
TEST_F(FilterTestBase, MyFilter)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    MyFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
}
```

## Стиль кода

### Общие правила

1. **Именование**:
   - Классы: `PascalCase` (например, `ImageProcessor`)
   - Функции и методы: `camelCase` (например, `loadFromFile`)
   - Константы: `UPPER_SNAKE_CASE` (например, `MAX_BUFFER_SIZE`)
   - Приватные члены: `snake_case_` с подчеркиванием в конце (например, `width_`)

2. **Форматирование**:
   - Используйте 4 пробела для отступов
   - Открывающая фигурная скобка на той же строке
   - Закрывающая фигурная скобка на отдельной строке

3. **Документация**:
   - Все публичные методы должны иметь Doxygen комментарии
   - Используйте `@brief`, `@param`, `@return`, `@note`

4. **Запрещено**:
   - `using namespace` в заголовочных файлах
   - Относительные пути в `#include`
   - Статические методы в приватном API (используйте anonymous namespace в .cpp)

### Примеры

```cpp
/**
 * @brief Загружает изображение из файла
 * @param filename Путь к файлу изображения
 * @param preserve_alpha Если true, загружает с альфа-каналом
 * @return true если загрузка успешна
 */
bool loadFromFile(const std::string& filename, bool preserve_alpha = false);
```

## Процесс разработки

### Рабочий процесс

1. Создайте ветку для новой функции
2. Реализуйте функцию с тестами
3. Убедитесь, что все тесты проходят
4. Проверьте код статическим анализатором
5. Создайте pull request

### Коммиты

Используйте понятные сообщения коммитов:
- `feat: добавлен фильтр X`
- `fix: исправлена ошибка в Y`
- `refactor: рефакторинг Z`
- `test: добавлены тесты для X`

## Тестирование

### Типы тестов

1. **Unit-тесты**: Тестирование отдельных компонентов
2. **Интеграционные тесты**: Тестирование взаимодействия компонентов
3. **Тесты граничных случаев**: Очень большие/маленькие изображения, невалидные данные

### Запуск тестов

```bash
cd build
cmake .. -DBUILD_TESTS=ON
cmake --build .
ctest
# или
./bin/ImageFilterTests
```

### Покрытие кода

Цель: 80%+ покрытие кода тестами.

## Дополнительные ресурсы

- [API документация](api/html/index.html) - Полная документация API
- [README.md](../README.md) - Общая информация о проекте
- [Примеры использования](../examples/) - Примеры кода

