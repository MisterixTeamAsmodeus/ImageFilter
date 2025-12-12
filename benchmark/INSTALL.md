# Инструкция по установке

## Вариант 1: Установка Poetry (рекомендуется)

### Шаг 1: Установка Poetry

Poetry можно установить несколькими способами:

#### Способ A: Через официальный установщик (Windows PowerShell)

```powershell
(Invoke-WebRequest -Uri https://install.python-poetry.org -UseBasicParsing).Content | python -
```

Если команда `python` не работает, попробуйте:

```powershell
(Invoke-WebRequest -Uri https://install.python-poetry.org -UseBasicParsing).Content | python3 -
```

#### Способ B: Через pip (если Python установлен)

```bash
pip install poetry
```

или

```bash
pip install --user poetry
```

#### Способ C: Через pipx (рекомендуется для изоляции)

```bash
pipx install poetry
```

### Шаг 2: Добавление Poetry в PATH

После установки через официальный установщик, Poetry обычно устанавливается в:
- Windows: `%APPDATA%\Python\Scripts` или `%LOCALAPPDATA%\pypoetry\Cache\poetry\bin`

Добавьте этот путь в переменную окружения PATH или используйте полный путь.

### Шаг 3: Проверка установки

```bash
poetry --version
```

### Шаг 4: Установка зависимостей проекта

```bash
cd benchmark
poetry install
```

## Вариант 2: Использование pip напрямую (без Poetry)

Если Poetry установить не удается, можно использовать pip напрямую:

### Шаг 1: Создание виртуального окружения

```bash
cd benchmark
python -m venv venv
```

### Шаг 2: Активация виртуального окружения

**Windows PowerShell:**
```powershell
.\venv\Scripts\Activate.ps1
```

**Windows CMD:**
```cmd
venv\Scripts\activate.bat
```

### Шаг 3: Установка зависимостей

```bash
pip install Pillow>=10.0.0 numpy>=1.24.0
```

### Шаг 4: Использование скриптов

```bash
# Генерация тестовых изображений
python -m benchmark.generate_test_images

# Запуск бенчмарка
python -m benchmark.benchmark
```

## Проверка установки Python

Если команды `python` или `python3` не работают:

1. Проверьте, установлен ли Python:
   - Скачайте с https://www.python.org/downloads/
   - При установке обязательно отметьте "Add Python to PATH"

2. После установки перезапустите терминал/PowerShell

3. Проверьте установку:
   ```bash
   python --version
   ```

## Быстрая установка (скрипт)

Создан скрипт `install.ps1` для автоматической установки. Запустите его из PowerShell:

```powershell
cd benchmark
.\install.ps1
```

