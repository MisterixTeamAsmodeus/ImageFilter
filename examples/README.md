# Примеры использования ImageFilter API

Эта директория содержит примеры использования API библиотеки ImageFilter для различных сценариев.

## Доступные примеры

### example_basic_usage.cpp
Базовый пример использования `ImageProcessor` для загрузки и сохранения изображений.

**Демонстрирует:**
- Загрузку изображения из файла
- Проверку валидности загруженного изображения
- Получение информации о размере изображения
- Сохранение изображения в файл
- Настройку качества JPEG

**Использование:**
```bash
# Скомпилируйте пример (требуется собрать проект)
g++ -std=c++20 -I../include example_basic_usage.cpp -o example_basic_usage \
    -L../build -lImageFilterLib

# Запустите (требуется файл input.jpg)
./example_basic_usage
```

### example_filter_usage.cpp
Пример применения фильтров к изображению через `FilterFactory`.

**Демонстрирует:**
- Создание фильтров через `FilterFactory`
- Применение одного фильтра к изображению
- Применение цепочки фильтров
- Обработку ошибок при применении фильтров

**Использование:**
```bash
g++ -std=c++20 -I../include example_filter_usage.cpp -o example_filter_usage \
    -L../build -lImageFilterLib

./example_filter_usage
```

### example_parallel_processing.cpp
Пример использования `ParallelImageProcessor` для распараллеливания обработки изображений.

**Демонстрирует:**
- Использование `ParallelImageProcessor` для распараллеливания обработки
- Обработку изображения построчно в нескольких потоках
- Адаптивный выбор режима обработки (последовательный/параллельный)
- Получение оптимального количества потоков

**Использование:**
```bash
g++ -std=c++20 -I../include example_parallel_processing.cpp -o example_parallel_processing \
    -L../build -lImageFilterLib -pthread

./example_parallel_processing
```

### example_custom_filter.cpp
Пример создания пользовательского фильтра, наследующегося от `IFilter`.

**Демонстрирует:**
- Создание пользовательского фильтра, наследующегося от `IFilter`
- Реализацию метода `apply()`
- Реализацию методов `getName()`, `getDescription()`, `getCategory()`
- Использование `FilterResult` для обработки ошибок
- Поддержку in-place обработки

**Использование:**
```bash
g++ -std=c++20 -I../include example_custom_filter.cpp -o example_custom_filter \
    -L../build -lImageFilterLib

./example_custom_filter
```

## Компиляция примеров

Для компиляции примеров необходимо:

1. Собрать основной проект:
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

2. Скомпилировать примеры с правильными путями к заголовочным файлам и библиотекам.

## Примечания

- Все примеры требуют наличия файла `input.jpg` в текущей директории
- Примеры создают выходные файлы с префиксом `output_`
- Убедитесь, что у вас есть права на чтение входных файлов и запись выходных файлов

## Дополнительная информация

Более подробная документация доступна в сгенерированной Doxygen документации:
```bash
cd build
cmake .. -DBUILD_DOCS=ON
cmake --build . --target docs
```

Документация будет доступна в `build/docs/doxygen/html/index.html`.

