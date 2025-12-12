# Скрипт установки зависимостей для бенчмарка
# Запуск: .\install.ps1

Write-Host "Проверка установки Python..." -ForegroundColor Cyan

# Проверяем наличие Python
$pythonCmd = $null
if (Get-Command python -ErrorAction SilentlyContinue) {
    $pythonCmd = "python"
} elseif (Get-Command python3 -ErrorAction SilentlyContinue) {
    $pythonCmd = "python3"
} elseif (Get-Command py -ErrorAction SilentlyContinue) {
    $pythonCmd = "py"
} else {
    Write-Host "ОШИБКА: Python не найден!" -ForegroundColor Red
    Write-Host "Пожалуйста, установите Python с https://www.python.org/downloads/" -ForegroundColor Yellow
    Write-Host "При установке обязательно отметьте 'Add Python to PATH'" -ForegroundColor Yellow
    exit 1
}

$pythonVersion = & $pythonCmd --version 2>&1
Write-Host "Найден: $pythonVersion" -ForegroundColor Green

# Проверяем наличие Poetry
Write-Host "`nПроверка установки Poetry..." -ForegroundColor Cyan
if (Get-Command poetry -ErrorAction SilentlyContinue) {
    Write-Host "Poetry уже установлен!" -ForegroundColor Green
    Write-Host "Установка зависимостей через Poetry..." -ForegroundColor Cyan
    poetry install
    if ($LASTEXITCODE -eq 0) {
        Write-Host "`nУстановка завершена успешно!" -ForegroundColor Green
        Write-Host "Используйте: poetry run benchmark" -ForegroundColor Yellow
        exit 0
    }
}

# Если Poetry не установлен, предлагаем установить
Write-Host "Poetry не найден. Установка Poetry..." -ForegroundColor Yellow

try {
    Write-Host "Скачивание установщика Poetry..." -ForegroundColor Cyan
    $installScript = (Invoke-WebRequest -Uri https://install.python-poetry.org -UseBasicParsing).Content
    
    Write-Host "Запуск установщика..." -ForegroundColor Cyan
    $installScript | & $pythonCmd -
    
    # Пытаемся найти Poetry после установки
    $poetryPath = "$env:APPDATA\Python\Scripts\poetry.exe"
    if (-not (Test-Path $poetryPath)) {
        $poetryPath = "$env:LOCALAPPDATA\pypoetry\Cache\poetry\bin\poetry.exe"
    }
    
    if (Test-Path $poetryPath) {
        Write-Host "Poetry установлен в: $poetryPath" -ForegroundColor Green
        Write-Host "Добавьте этот путь в PATH или используйте полный путь" -ForegroundColor Yellow
        Write-Host "Или перезапустите терминал и попробуйте снова" -ForegroundColor Yellow
    } else {
        Write-Host "Poetry установлен, но не найден в стандартных путях" -ForegroundColor Yellow
        Write-Host "Попробуйте перезапустить терминал" -ForegroundColor Yellow
    }
    
    # Альтернатива: установка через pip
    Write-Host "`nАльтернатива: установка Poetry через pip..." -ForegroundColor Cyan
    & $pythonCmd -m pip install --user poetry
    
    if (Get-Command poetry -ErrorAction SilentlyContinue) {
        Write-Host "Poetry установлен через pip!" -ForegroundColor Green
        poetry install
        if ($LASTEXITCODE -eq 0) {
            Write-Host "`nУстановка завершена успешно!" -ForegroundColor Green
            exit 0
        }
    }
    
} catch {
    Write-Host "Ошибка при установке Poetry: $_" -ForegroundColor Red
}

# Если Poetry не удалось установить, используем pip напрямую
Write-Host "`nУстановка зависимостей через pip (без Poetry)..." -ForegroundColor Cyan

# Создаем виртуальное окружение
if (-not (Test-Path "venv")) {
    Write-Host "Создание виртуального окружения..." -ForegroundColor Cyan
    & $pythonCmd -m venv venv
}

# Активируем виртуальное окружение
Write-Host "Активация виртуального окружения..." -ForegroundColor Cyan
& .\venv\Scripts\Activate.ps1

# Устанавливаем зависимости
Write-Host "Установка зависимостей..." -ForegroundColor Cyan
pip install Pillow>=10.0.0 numpy>=1.24.0

if ($LASTEXITCODE -eq 0) {
    Write-Host "`nУстановка завершена успешно!" -ForegroundColor Green
    Write-Host "Используйте:" -ForegroundColor Yellow
    Write-Host "  .\venv\Scripts\Activate.ps1" -ForegroundColor Yellow
    Write-Host "  python -m imagefilter_benchmark.benchmark" -ForegroundColor Yellow
} else {
    Write-Host "Ошибка при установке зависимостей" -ForegroundColor Red
    exit 1
}

