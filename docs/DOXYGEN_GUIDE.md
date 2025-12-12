# Руководство по генерации Doxygen документации

Это руководство описывает процесс генерации API документации для проекта ImageFilter с помощью Doxygen.

## Установка Doxygen

### macOS
```bash
brew install doxygen
```

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install doxygen
```

### Windows
```bash
choco install doxygen.install
```

Или скачайте установщик с [официального сайта Doxygen](https://www.doxygen.nl/download.html).

## Генерация документации

### Способ 1: Через CMake (рекомендуется)

1. Соберите проект с опцией `BUILD_DOCS`:
```bash
mkdir build
cd build
cmake .. -DBUILD_DOCS=ON
cmake --build . --target docs
```

2. Документация будет сгенерирована в `build/docs/doxygen/html/index.html`

### Способ 2: Напрямую через Doxygen

1. Убедитесь, что вы находитесь в корневой директории проекта:
```bash
cd /path/to/ImageFilter
```

2. Запустите Doxygen:
```bash
doxygen Doxyfile
```

3. Документация будет сгенерирована в `docs/doxygen/html/index.html`

## Просмотр документации

Откройте файл `docs/doxygen/html/index.html` в вашем браузере.

## Структура документации

Сгенерированная документация включает:

- **Главная страница** - Обзор проекта и быстрый старт
- **Классы** - Документация всех классов API
- **Файлы** - Документация исходных файлов
- **Примеры** - Примеры использования API из директории `examples/`
- **Диаграммы** - Диаграммы классов и зависимостей

## Примеры кода

Примеры кода находятся в директории `examples/` и автоматически включаются в документацию:

- `example_basic_usage.cpp` - Базовое использование ImageProcessor
- `example_filter_usage.cpp` - Применение фильтров
- `example_parallel_processing.cpp` - Параллельная обработка
- `example_custom_filter.cpp` - Создание пользовательского фильтра

## Настройка Doxygen

Конфигурация Doxygen находится в файле `Doxyfile` в корневой директории проекта.

Основные настройки:
- `PROJECT_NAME` - Название проекта
- `OUTPUT_DIRECTORY` - Директория для выходных файлов
- `INPUT` - Входные директории и файлы
- `EXAMPLE_PATH` - Путь к примерам кода
- `OUTPUT_LANGUAGE` - Язык документации (Russian)

## Обновление документации

После изменения кода или добавления новых примеров:

1. Обновите комментарии в коде (если необходимо)
2. Перегенерируйте документацию:
```bash
cd build
cmake --build . --target docs
```

## Решение проблем

### Doxygen не найден

Убедитесь, что Doxygen установлен и доступен в PATH:
```bash
which doxygen
doxygen --version
```

### Ошибки при генерации

Проверьте файл `Doxyfile` на наличие синтаксических ошибок. Doxygen выведет предупреждения и ошибки в консоль.

### Примеры не отображаются

Убедитесь, что:
- Директория `examples/` существует и содержит файлы с расширением `.cpp`
- В файлах примеров есть тег `@example`
- `EXAMPLE_PATH` в `Doxyfile` указывает на правильную директорию

## Дополнительные ресурсы

- [Официальная документация Doxygen](https://www.doxygen.nl/manual/index.html)
- [Руководство пользователя](USER_GUIDE.md)
- [Руководство разработчика](DEVELOPER_GUIDE.md)

