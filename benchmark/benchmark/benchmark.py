#!/usr/bin/env python3
"""
Бенчмарк тест для измерения времени выполнения фильтров изображений.

Запускает C++ приложение ImageFilter для каждого фильтра и изображения,
измеряет время выполнения и выводит статистику.
"""

import os
import subprocess
import time
import csv
import shutil
import itertools
from pathlib import Path
from typing import List, Dict, Tuple
from dataclasses import dataclass
from statistics import mean, median, stdev


@dataclass
class BenchmarkResult:
    """Результат одного теста бенчмарка."""
    filter_name: str
    image_path: str
    image_size: Tuple[int, int]  # (width, height)
    execution_time: float  # в секундах
    success: bool
    error_message: str = ""
    is_chain: bool = False  # True если это цепочка фильтров
    chain_length: int = 1  # Количество фильтров в цепочке


class ImageFilterBenchmark:
    """Класс для проведения бенчмарка фильтров изображений."""
    
    def __init__(self, executable_path: str, dataset_dir: str = "dataset", output_dir: str = "benchmark_output"):
        """
        Инициализирует бенчмарк.
        
        Args:
            executable_path: Путь к исполняемому файлу ImageFilter
            dataset_dir: Директория с тестовыми изображениями
            output_dir: Директория для сохранения результатов
        """
        self.executable_path = Path(executable_path)
        self.dataset_dir = Path(dataset_dir)
        self.output_dir = Path(output_dir)
        self.statistics_dir = Path(output_dir) / "statistics"
        self.results: List[BenchmarkResult] = []
        
        # Доступные фильтры, сгруппированные по категориям
        # Цветовые фильтры
        color_filters = [
            "grayscale",  # Преобразование в оттенки серого
            "sepia",      # Эффект сепии
            "invert",     # Инверсия цветов
            "brightness", # Изменение яркости
            "contrast",   # Изменение контрастности
            "saturation", # Изменение насыщенности
        ]
        
        # Геометрические фильтры
        geometric_filters = [
            "flip_h",     # Горизонтальное отражение
            "flip_v",     # Вертикальное отражение
            "rotate90",   # Поворот на 90 градусов
        ]
        
        # Фильтры краёв и деталей
        edge_filters = [
            "sharpen",    # Повышение резкости
            "edges",      # Детекция краёв
            "emboss",     # Эффект рельефа
            "outline",    # Выделение контуров
        ]
        
        # Фильтры размытия и шума
        blur_filters = [
            "blur",       # Размытие по Гауссу
            "box_blur",   # Размытие по прямоугольнику
            "motion_blur", # Размытие движения
            "median",    # Медианный фильтр
            "noise",     # Добавление шума
        ]
        
        # Стилистические фильтры
        style_filters = [
            "posterize",  # Постеризация
            "threshold",  # Пороговая бинаризация
            "vignette",  # Виньетирование
        ]
        
        # Объединяем все фильтры
        self.filters = (
            color_filters +
            geometric_filters +
            edge_filters +
            blur_filters +
            style_filters
        )
        
        # Сохраняем категории для группировки статистики
        self.filter_categories = {
            "Цветовые": color_filters,
            "Геометрические": geometric_filters,
            "Края и детали": edge_filters,
            "Размытие и шум": blur_filters,
            "Стилистические": style_filters,
        }
        
        # Определяем типичные цепочки фильтров для бенчмарка
        self.filter_chains = [
            # Простые цепочки (2-3 фильтра)
            ("grayscale,sharpen", "Черно-белое + резкость"),
            ("sepia,vignette", "Сепия + виньетирование"),
            ("brightness,contrast", "Яркость + контраст"),
            ("blur,noise", "Размытие + шум"),
            ("invert,posterize", "Инверсия + постеризация"),
            
            # Средние цепочки (3-4 фильтра)
            ("grayscale,sharpen,vignette", "Черно-белое + резкость + виньетирование"),
            ("brightness,contrast,saturation", "Яркость + контраст + насыщенность"),
            ("blur,median,sharpen", "Размытие + медианный фильтр + резкость"),
            ("sepia,blur,vignette", "Сепия + размытие + виньетирование"),
            ("grayscale,edges,threshold", "Черно-белое + края + порог"),
            
            # Сложные цепочки (4-5 фильтров)
            ("grayscale,sharpen,contrast,vignette", "Черно-белое + резкость + контраст + виньетирование"),
            ("brightness,contrast,saturation,sharpen", "Яркость + контраст + насыщенность + резкость"),
            ("blur,median,sharpen,edges", "Размытие + медианный + резкость + края"),
            ("sepia,blur,vignette,posterize", "Сепия + размытие + виньетирование + постеризация"),
            ("grayscale,invert,sharpen,threshold", "Черно-белое + инверсия + резкость + порог"),
            
            # Очень сложные цепочки (5+ фильтров)
            ("grayscale,sharpen,contrast,vignette,posterize", "Черно-белое + резкость + контраст + виньетирование + постеризация"),
            ("brightness,contrast,saturation,sharpen,edges", "Яркость + контраст + насыщенность + резкость + края"),
            ("blur,median,sharpen,edges,threshold", "Размытие + медианный + резкость + края + порог"),
        ]
        
        # Удаляем выходную директорию, если она существует, и создаем заново
        if self.output_dir.exists():
            shutil.rmtree(self.output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)
        # Создаем отдельную директорию для статистики
        self.statistics_dir.mkdir(parents=True, exist_ok=True)
        
        if not self.executable_path.exists():
            raise FileNotFoundError(f"Исполняемый файл не найден: {executable_path}")
        
        if not self.dataset_dir.exists():
            raise FileNotFoundError(f"Директория с датасетом не найдена: {dataset_dir}")
    
    def get_available_filters(self) -> List[str]:
        """
        Получает актуальный список всех доступных фильтров.
        
        Сначала пытается получить список из CLI приложения через --list-filters,
        если не удается, использует список фильтров по умолчанию.
        
        Returns:
            Список имен фильтров
        """
        try:
            # Пытаемся получить список фильтров через --list-filters
            result = subprocess.run(
                [str(self.executable_path), "--list-filters"],
                capture_output=True,
                text=True,
                encoding='utf-8',
                errors='replace',
                timeout=10
            )
            
            if result.returncode == 0:
                # Парсим вывод --list-filters
                filters = []
                in_category = False
                for line in result.stdout.split('\n'):
                    line = line.strip()
                    # Пропускаем заголовки и пустые строки
                    if not line or line == "Доступные фильтры:" or line.endswith(':'):
                        in_category = line.endswith(':')
                        continue
                    
                    # Ищем строки с именами фильтров (формат: "  filter_name   - описание")
                    if in_category or (line and ' - ' in line):
                        # Извлекаем имя фильтра (первое слово до " - ")
                        if ' - ' in line:
                            filter_name = line.split(' - ')[0].strip()
                        else:
                            parts = line.split()
                            filter_name = parts[0] if parts else ""
                        
                        # Проверяем, что это валидное имя фильтра
                        if filter_name and not filter_name.startswith('-') and len(filter_name) > 1:
                            # Проверяем, что это не описание (не содержит пробелы или специальные символы)
                            if all(c.isalnum() or c == '_' for c in filter_name):
                                filters.append(filter_name)
                
                if filters:
                    unique_filters = sorted(list(set(filters)))  # Убираем дубликаты и сортируем
                    print(f"Получено {len(unique_filters)} фильтров из CLI приложения")
                    return unique_filters
        except Exception as e:
            print(f"Предупреждение: не удалось получить список фильтров из CLI: {e}")
            print("Используется список фильтров по умолчанию")
        
        # Возвращаем список фильтров по умолчанию
        default_filters = sorted(list(self.filters))
        print(f"Используется список из {len(default_filters)} фильтров по умолчанию")
        return default_filters
    
    def generate_all_combinations(self, max_chain_length: int = None) -> List[Tuple[str, int]]:
        """
        Генерирует все возможные комбинации фильтров для цепочек разной длины.
        Порядок фильтров не важен (A,B и B,A считаются одной комбинацией).
        
        Args:
            max_chain_length: Максимальная длина цепочки (None = все фильтры)
        
        Returns:
            Список кортежей (цепочка_фильтров, длина_цепочки)
        """
        available_filters = self.get_available_filters()
        total_filters = len(available_filters)
        
        if max_chain_length is None:
            max_chain_length = total_filters
        
        # Ограничиваем максимальную длину для предотвращения переполнения
        if max_chain_length > total_filters:
            max_chain_length = total_filters
        
        # Предупреждаем, если будет слишком много комбинаций
        total_combinations = 0
        import math
        for length in range(1, max_chain_length + 1):
            # Количество комбинаций: C(n, k) = n! / (k! * (n-k)!)
            if length <= total_filters:
                # math.comb доступен в Python 3.8+
                if hasattr(math, 'comb'):
                    comb_count = math.comb(total_filters, length)
                else:
                    # Fallback для старых версий Python
                    from math import factorial
                    comb_count = factorial(total_filters) // (factorial(length) * factorial(total_filters - length))
                total_combinations += comb_count
        
        print(f"Генерация всех комбинаций фильтров (порядок не важен):")
        print(f"  Всего фильтров: {total_filters}")
        print(f"  Максимальная длина цепочки: {max_chain_length}")
        print(f"  Всего комбинаций: {total_combinations:,}")
        
        if total_combinations > 100000:
            print(f"  ⚠️  ВНИМАНИЕ: Будет сгенерировано очень много комбинаций!")
            print(f"     Рекомендуется ограничить max_chain_length до 3-4 фильтров")
        
        combinations = []
        
        # Генерируем комбинации для каждой длины от 1 до max_chain_length
        for chain_length in range(1, max_chain_length + 1):
            # Генерируем все комбинации длины chain_length (порядок не важен)
            for combo in itertools.combinations(available_filters, chain_length):
                # Создаем строку цепочки через запятую (сортируем для консистентности)
                sorted_combo = sorted(combo)  # Сортируем для единообразия
                chain_str = ",".join(sorted_combo)
                combinations.append((chain_str, chain_length))
        
        return combinations
    
    def get_image_size(self, image_path: Path) -> Tuple[int, int]:
        """
        Получает размер изображения.
        
        Args:
            image_path: Путь к изображению
        
        Returns:
            Кортеж (width, height)
        """
        try:
            from PIL import Image
            with Image.open(image_path) as img:
                return img.size
        except Exception:
            return (0, 0)
    
    def run_filter(self, image_path: Path, filter_name: str, iterations: int = 1) -> BenchmarkResult:
        """
        Запускает фильтр на изображении и измеряет время выполнения.
        
        Args:
            image_path: Путь к входному изображению
            filter_name: Имя фильтра
            iterations: Количество итераций для усреднения
        
        Returns:
            BenchmarkResult с результатами
        """
        output_path = self.output_dir / f"{image_path.stem}_{filter_name}.jpg"
        
        # Команда для запуска
        cmd = [
            str(self.executable_path),
            str(image_path),
            filter_name,
            str(output_path)
        ]
        
        execution_times = []
        success = False
        error_message = ""
        
        for _ in range(iterations):
            try:
                start_time = time.perf_counter()
                result = subprocess.run(
                    cmd,
                    capture_output=True,
                    text=True,
                    encoding='utf-8',
                    errors='replace',  # Заменяем недопустимые символы вместо ошибки
                    timeout=300  # 5 минут максимум
                )
                end_time = time.perf_counter()
                
                execution_time = end_time - start_time
                execution_times.append(execution_time)
                
                if result.returncode == 0:
                    success = True
                else:
                    # Безопасное извлечение сообщения об ошибке
                    if result.stderr:
                        error_message = result.stderr
                    elif result.stdout:
                        error_message = result.stdout
                    else:
                        error_message = f"Процесс завершился с кодом {result.returncode}"
                    
            except subprocess.TimeoutExpired:
                error_message = "Timeout (превышено 5 минут)"
                break
            except UnicodeDecodeError as e:
                error_message = f"Ошибка кодировки при чтении вывода: {e}"
                break
            except Exception as e:
                error_message = str(e)
                break
        
        avg_time = mean(execution_times) if execution_times else 0.0
        image_size = self.get_image_size(image_path)
        
        return BenchmarkResult(
            filter_name=filter_name,
            image_path=str(image_path),
            image_size=image_size,
            execution_time=avg_time,
            success=success,
            error_message=error_message,
            is_chain=False,
            chain_length=1
        )
    
    def run_benchmark(self, iterations: int = 3, image_pattern: str = "*.jpg") -> None:
        """
        Запускает полный бенчмарк для всех фильтров и изображений.
        
        Args:
            iterations: Количество итераций для каждого теста
            image_pattern: Паттерн для поиска изображений (например, "*.jpg")
        """
        # Находим все изображения в датасете
        image_files = list(self.dataset_dir.glob(image_pattern))
        
        if not image_files:
            print(f"Предупреждение: изображения не найдены в {self.dataset_dir}")
            return
        
        print(f"Найдено {len(image_files)} изображений")
        print(f"Тестируем {len(self.filters)} фильтров")
        print(f"Всего тестов: {len(image_files) * len(self.filters)}")
        print(f"Итераций на тест: {iterations}")
        print("-" * 80)
        
        total_tests = len(image_files) * len(self.filters)
        current_test = 0
        
        for image_path in sorted(image_files):
            for filter_name in self.filters:
                current_test += 1
                print(f"[{current_test}/{total_tests}] {image_path.name} -> {filter_name}...", end=" ", flush=True)
                
                result = self.run_filter(image_path, filter_name, iterations)
                self.results.append(result)
                
                if result.success:
                    print(f"✓ {result.execution_time:.4f}s")
                else:
                    print(f"✗ Ошибка: {result.error_message}")
        
        print("-" * 80)
        print("Бенчмарк завершен!")
    
    def print_statistics(self) -> None:
        """Выводит статистику результатов бенчмарка."""
        if not self.results:
            print("Нет результатов для анализа")
            return
        
        successful_results = [r for r in self.results if r.success]
        
        if not successful_results:
            print("Нет успешных результатов")
            return
        
        print("\n" + "=" * 80)
        print("СТАТИСТИКА БЕНЧМАРКА")
        print("=" * 80)
        
        # Разделяем результаты на одиночные фильтры и цепочки
        single_filter_results = [r for r in successful_results if not r.is_chain]
        chain_results = [r for r in successful_results if r.is_chain]
        
        # Общая статистика
        print(f"\nОбщая статистика:")
        print(f"  Всего тестов: {len(self.results)}")
        print(f"  Успешных: {len(successful_results)}")
        print(f"  Неудачных: {len(self.results) - len(successful_results)}")
        print(f"  Одиночных фильтров: {len(single_filter_results)}")
        print(f"  Цепочек фильтров: {len(chain_results)}")
        print(f"  Всего фильтров: {len(self.filters)}")
        for category_name, category_filters in self.filter_categories.items():
            category_success = len([r for r in single_filter_results if r.filter_name in category_filters])
            print(f"    {category_name}: {len(category_filters)} фильтров ({category_success} успешных)")
        
        # Группируем по категориям фильтров (только одиночные фильтры)
        for category_name, category_filters in self.filter_categories.items():
            category_results = [r for r in single_filter_results if r.filter_name in category_filters]
            
            if not category_results:
                continue
            
            print(f"\n{category_name} фильтры:")
            print("-" * 80)
            
            # Группируем по фильтрам внутри категории
            for filter_name in category_filters:
                filter_results = [r for r in category_results if r.filter_name == filter_name]
                
                if not filter_results:
                    continue
                
                times = [r.execution_time for r in filter_results]
                pixels = [r.image_size[0] * r.image_size[1] for r in filter_results]
                
                print(f"\n  {filter_name}:")
                print(f"    Количество тестов: {len(filter_results)}")
                print(f"    Среднее время: {mean(times):.4f}s")
                print(f"    Медианное время: {median(times):.4f}s")
                if len(times) > 1:
                    print(f"    Стандартное отклонение: {stdev(times):.4f}s")
                print(f"    Минимальное время: {min(times):.4f}s")
                print(f"    Максимальное время: {max(times):.4f}s")
                
                # Производительность в мегапикселях в секунду
                if pixels:
                    avg_pixels = mean(pixels)
                    avg_time = mean(times)
                    if avg_time > 0:
                        mp_per_sec = (avg_pixels / 1_000_000) / avg_time
                        print(f"    Производительность: {mp_per_sec:.2f} MP/s")
        
        # Группируем по размерам изображений
        print("\n" + "-" * 80)
        print("Производительность по размерам изображений:")
        print("-" * 80)
        
        size_groups = {}
        for result in successful_results:
            size_key = f"{result.image_size[0]}x{result.image_size[1]}"
            if size_key not in size_groups:
                size_groups[size_key] = []
            size_groups[size_key].append(result)
        
        for size_key in sorted(size_groups.keys(), key=lambda x: int(x.split('x')[0]) * int(x.split('x')[1])):
            results = size_groups[size_key]
            times = [r.execution_time for r in results]
            print(f"\n{size_key}:")
            print(f"  Среднее время: {mean(times):.4f}s")
            print(f"  По фильтрам:")
            for filter_name in self.filters:
                filter_times = [r.execution_time for r in results if r.filter_name == filter_name and not r.is_chain]
                if filter_times:
                    print(f"    {filter_name}: {mean(filter_times):.4f}s")
        
        # Статистика по цепочкам фильтров
        if chain_results:
            print("\n" + "=" * 80)
            print("СТАТИСТИКА ПО ЦЕПОЧКАМ ФИЛЬТРОВ")
            print("=" * 80)
            
            # Группируем по длине цепочки
            chain_by_length = {}
            for result in chain_results:
                length = result.chain_length
                if length not in chain_by_length:
                    chain_by_length[length] = []
                chain_by_length[length].append(result)
            
            for length in sorted(chain_by_length.keys()):
                results = chain_by_length[length]
                times = [r.execution_time for r in results]
                pixels = [r.image_size[0] * r.image_size[1] for r in results]
                
                print(f"\nЦепочки из {length} фильтров:")
                print(f"  Количество тестов: {len(results)}")
                print(f"  Среднее время: {mean(times):.4f}s")
                print(f"  Медианное время: {median(times):.4f}s")
                if len(times) > 1:
                    print(f"  Стандартное отклонение: {stdev(times):.4f}s")
                print(f"  Минимальное время: {min(times):.4f}s")
                print(f"  Максимальное время: {max(times):.4f}s")
                
                if pixels and mean(times) > 0:
                    avg_pixels = mean(pixels)
                    mp_per_sec = (avg_pixels / 1_000_000) / mean(times)
                    print(f"  Производительность: {mp_per_sec:.2f} MP/s")
                    avg_time_per_filter = mean(times) / length
                    print(f"  Среднее время на фильтр: {avg_time_per_filter:.4f}s")
            
            # Группируем по конкретным цепочкам
            print("\n" + "-" * 80)
            print("Производительность по цепочкам:")
            print("-" * 80)
            
            chain_groups = {}
            for result in chain_results:
                chain_name = result.filter_name
                if chain_name not in chain_groups:
                    chain_groups[chain_name] = []
                chain_groups[chain_name].append(result)
            
            for chain_name in sorted(chain_groups.keys()):
                results = chain_groups[chain_name]
                times = [r.execution_time for r in results]
                pixels = [r.image_size[0] * r.image_size[1] for r in results]
                
                # Находим описание цепочки
                chain_desc = ""
                for chain_str, desc in self.filter_chains:
                    if chain_str == chain_name:
                        chain_desc = desc
                        break
                
                print(f"\n  {chain_name} ({chain_desc}):")
                print(f"    Количество тестов: {len(results)}")
                print(f"    Среднее время: {mean(times):.4f}s")
                if pixels and mean(times) > 0:
                    avg_pixels = mean(pixels)
                    mp_per_sec = (avg_pixels / 1_000_000) / mean(times)
                    print(f"    Производительность: {mp_per_sec:.2f} MP/s")
                    avg_time_per_filter = mean(times) / results[0].chain_length
                    print(f"    Среднее время на фильтр: {avg_time_per_filter:.4f}s")
    
    def save_results_csv(self, filename: str = "benchmark_results.csv") -> None:
        """Сохраняет результаты в CSV файл."""
        csv_path = self.statistics_dir / filename
        
        with open(csv_path, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow([
                "Фильтр/Цепочка", "Тип", "Длина цепочки", "Изображение", "Ширина", "Высота", 
                "Пикселей", "Время (с)", "Успех", "Ошибка"
            ])
            
            for result in self.results:
                pixels = result.image_size[0] * result.image_size[1]
                filter_type = "Цепочка" if result.is_chain else "Одиночный"
                writer.writerow([
                    result.filter_name,
                    filter_type,
                    result.chain_length,
                    Path(result.image_path).name,
                    result.image_size[0],
                    result.image_size[1],
                    pixels,
                    f"{result.execution_time:.6f}",
                    "Да" if result.success else "Нет",
                    result.error_message
                ])
        
        print(f"\nРезультаты сохранены в: {csv_path}")
    
    def run_filter_chain(self, image_path: Path, filter_chain: str, iterations: int = 1) -> BenchmarkResult:
        """
        Запускает цепочку фильтров на изображении и измеряет время выполнения.
        
        Args:
            image_path: Путь к входному изображению
            filter_chain: Цепочка фильтров, разделенная запятыми (например, "grayscale,sharpen,vignette")
            iterations: Количество итераций для усреднения
        
        Returns:
            BenchmarkResult с результатами
        """
        # Создаем имя выходного файла на основе цепочки фильтров
        chain_name = filter_chain.replace(",", "_")
        output_path = self.output_dir / f"{image_path.stem}_chain_{chain_name}.jpg"
        
        # Команда для запуска (фильтры передаются через запятую)
        cmd = [
            str(self.executable_path),
            str(image_path),
            filter_chain,  # Цепочка фильтров через запятую
            str(output_path)
        ]
        
        execution_times = []
        success = False
        error_message = ""
        
        for _ in range(iterations):
            try:
                start_time = time.perf_counter()
                result = subprocess.run(
                    cmd,
                    capture_output=True,
                    text=True,
                    encoding='utf-8',
                    errors='replace',
                    timeout=600  # 10 минут максимум для цепочек
                )
                end_time = time.perf_counter()
                
                execution_time = end_time - start_time
                execution_times.append(execution_time)
                
                if result.returncode == 0:
                    success = True
                else:
                    if result.stderr:
                        error_message = result.stderr
                    elif result.stdout:
                        error_message = result.stdout
                    else:
                        error_message = f"Процесс завершился с кодом {result.returncode}"
                    
            except subprocess.TimeoutExpired:
                error_message = "Timeout (превышено 10 минут)"
                break
            except UnicodeDecodeError as e:
                error_message = f"Ошибка кодировки при чтении вывода: {e}"
                break
            except Exception as e:
                error_message = str(e)
                break
        
        avg_time = mean(execution_times) if execution_times else 0.0
        image_size = self.get_image_size(image_path)
        chain_length = len(filter_chain.split(","))
        
        return BenchmarkResult(
            filter_name=filter_chain,
            image_path=str(image_path),
            image_size=image_size,
            execution_time=avg_time,
            success=success,
            error_message=error_message,
            is_chain=True,
            chain_length=chain_length
        )
    
    def run_chain_benchmark(self, iterations: int = 3, image_pattern: str = "*.jpg") -> None:
        """
        Запускает бенчмарк для цепочек фильтров.
        
        Args:
            iterations: Количество итераций для каждого теста
            image_pattern: Паттерн для поиска изображений (например, "*.jpg")
        """
        # Находим все изображения в датасете
        image_files = list(self.dataset_dir.glob(image_pattern))
        
        if not image_files:
            print(f"Предупреждение: изображения не найдены в {self.dataset_dir}")
            return
        
        print(f"Найдено {len(image_files)} изображений")
        print(f"Тестируем {len(self.filter_chains)} цепочек фильтров")
        print(f"Всего тестов: {len(image_files) * len(self.filter_chains)}")
        print(f"Итераций на тест: {iterations}")
        print("-" * 80)
        
        total_tests = len(image_files) * len(self.filter_chains)
        current_test = 0
        
        for image_path in sorted(image_files):
            for chain_str, chain_description in self.filter_chains:
                current_test += 1
                print(f"[{current_test}/{total_tests}] {image_path.name} -> {chain_str} ({chain_description})...", 
                      end=" ", flush=True)
                
                result = self.run_filter_chain(image_path, chain_str, iterations)
                self.results.append(result)
                
                if result.success:
                    print(f"✓ {result.execution_time:.4f}s ({result.chain_length} фильтров)")
                else:
                    print(f"✗ Ошибка: {result.error_message}")
        
        print("-" * 80)
        print("Бенчмарк цепочек завершен!")
    
    def run_all_combinations_benchmark(self, 
                                      iterations: int = 3, 
                                      image_pattern: str = "*.jpg",
                                      max_chain_length: int = None,
                                      max_combinations_per_length: int = None) -> None:
        """
        Запускает бенчмарк для всех возможных комбинаций фильтров без учета порядка.
        
        Генерирует все комбинации фильтров для цепочек длиной от 1 до max_chain_length.
        Порядок фильтров не важен (A,B и B,A считаются одной комбинацией).
        
        Args:
            iterations: Количество итераций для каждого теста
            image_pattern: Паттерн для поиска изображений (например, "*.jpg")
            max_chain_length: Максимальная длина цепочки (None = все фильтры)
            max_combinations_per_length: Максимальное количество комбинаций для каждой длины
                                        (None = все комбинации, полезно для ограничения)
        """
        # Находим все изображения в датасете
        image_files = list(self.dataset_dir.glob(image_pattern))
        
        if not image_files:
            print(f"Предупреждение: изображения не найдены в {self.dataset_dir}")
            return
        
        # Генерируем все комбинации
        all_combinations = self.generate_all_combinations(max_chain_length)
        
        # Если указано ограничение на количество комбинаций для каждой длины
        if max_combinations_per_length is not None:
            # Группируем по длине и ограничиваем
            combinations_by_length = {}
            for chain_str, length in all_combinations:
                if length not in combinations_by_length:
                    combinations_by_length[length] = []
                combinations_by_length[length].append((chain_str, length))
            
            limited_combinations = []
            for length in sorted(combinations_by_length.keys()):
                combos = combinations_by_length[length]
                if len(combos) > max_combinations_per_length:
                    print(f"  Ограничение: для длины {length} выбрано {max_combinations_per_length} из {len(combos)} комбинаций")
                    limited_combinations.extend(combos[:max_combinations_per_length])
                else:
                    limited_combinations.extend(combos)
            
            all_combinations = limited_combinations
        
        print(f"\nНайдено {len(image_files)} изображений")
        print(f"Сгенерировано {len(all_combinations)} комбинаций фильтров")
        print(f"Всего тестов: {len(image_files) * len(all_combinations)}")
        print(f"Итераций на тест: {iterations}")
        print("-" * 80)
        
        total_tests = len(image_files) * len(all_combinations)
        current_test = 0
        
        # Группируем комбинации по длине для лучшего отображения прогресса
        combinations_by_length = {}
        for chain_str, length in all_combinations:
            if length not in combinations_by_length:
                combinations_by_length[length] = []
            combinations_by_length[length].append((chain_str, length))
        
        for image_path in sorted(image_files):
            # Тестируем комбинации по возрастанию длины
            for length in sorted(combinations_by_length.keys()):
                for chain_str, chain_length in combinations_by_length[length]:
                    current_test += 1
                    # Сокращаем отображение длинных цепочек
                    display_chain = chain_str if len(chain_str) <= 60 else chain_str[:57] + "..."
                    print(f"[{current_test}/{total_tests}] {image_path.name} -> {display_chain} ({chain_length} фильтров)...", 
                          end=" ", flush=True)
                    
                    result = self.run_filter_chain(image_path, chain_str, iterations)
                    self.results.append(result)
                    
                    if result.success:
                        print(f"✓ {result.execution_time:.4f}s")
                    else:
                        print(f"✗ Ошибка: {result.error_message}")
        
        print("-" * 80)
        print("Бенчмарк всех комбинаций завершен!")
    
    def save_statistics_csv(self, filename: str = "benchmark_statistics.csv") -> None:
        """
        Сохраняет статистику бенчмарка по фильтрам в CSV файл.
        
        Args:
            filename: Имя файла для сохранения статистики
        """
        if not self.results:
            return
        
        successful_results = [r for r in self.results if r.success]
        
        if not successful_results:
            return
        
        csv_path = self.statistics_dir / filename
        
        with open(csv_path, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow([
                "Фильтр/Цепочка",
                "Тип",
                "Длина цепочки",
                "Количество тестов",
                "Среднее время (с)",
                "Медианное время (с)",
                "Стандартное отклонение (с)",
                "Минимальное время (с)",
                "Максимальное время (с)",
                "Производительность (MP/s)",
                "Время на фильтр (с)"
            ])
            
            # Группируем результаты по фильтрам/цепочкам
            filter_groups = {}
            for result in successful_results:
                if result.filter_name not in filter_groups:
                    filter_groups[result.filter_name] = []
                filter_groups[result.filter_name].append(result)
            
            # Сортируем фильтры по имени для консистентности
            for filter_name in sorted(filter_groups.keys()):
                filter_results = filter_groups[filter_name]
                
                times = [r.execution_time for r in filter_results]
                pixels = [r.image_size[0] * r.image_size[1] for r in filter_results]
                
                # Вычисляем статистику
                count = len(filter_results)
                avg_time = mean(times)
                median_time = median(times)
                std_dev = stdev(times) if len(times) > 1 else 0.0
                min_time = min(times)
                max_time = max(times)
                
                # Производительность в мегапикселях в секунду
                if pixels and avg_time > 0:
                    avg_pixels = mean(pixels)
                    mp_per_sec = (avg_pixels / 1_000_000) / avg_time
                else:
                    mp_per_sec = 0.0
                
                # Определяем тип и длину цепочки
                is_chain = filter_results[0].is_chain
                chain_length = filter_results[0].chain_length if is_chain else 1
                filter_type = "Цепочка" if is_chain else "Одиночный"
                
                # Время на фильтр
                time_per_filter = avg_time / chain_length if chain_length > 0 else avg_time
                
                writer.writerow([
                    filter_name,
                    filter_type,
                    chain_length,
                    count,
                    f"{avg_time:.4f}",
                    f"{median_time:.4f}",
                    f"{std_dev:.4f}",
                    f"{min_time:.4f}",
                    f"{max_time:.4f}",
                    f"{mp_per_sec:.2f}",
                    f"{time_per_filter:.4f}"
                ])
        
        print(f"Статистика сохранена в: {csv_path}")


def main():
    """Главная функция для запуска бенчмарка."""
    import argparse
    
    # Определяем путь к исполняемому файлу относительно подпроекта
    project_root = Path(__file__).parent.parent.parent
    # Пробуем разные варианты имени исполняемого файла
    if (project_root / "cmake-build-debug" / "bin" / "ImageFilter").exists():
        default_executable = project_root / "cmake-build-debug" / "bin" / "ImageFilter"
    elif (project_root / "cmake-build-release" / "bin" / "ImageFilter").exists():
        default_executable = project_root / "cmake-build-release" / "bin" / "ImageFilter"
    elif (project_root / "build" / "bin" / "ImageFilter").exists():
        default_executable = project_root / "build" / "bin" / "ImageFilter"
    else:
        default_executable = project_root / "bin" / "ImageFilter"
    
    parser = argparse.ArgumentParser(
        description="Бенчмарк тест для фильтров изображений",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Примеры использования:
  poetry run benchmark
  poetry run benchmark --iterations 5
  poetry run benchmark --executable ../cmake-build-debug/bin/ImageFilter
  poetry run benchmark --dataset dataset --output benchmark_output
  poetry run benchmark --chains  # Только цепочки фильтров
  poetry run benchmark --both    # Одиночные фильтры + цепочки
  poetry run benchmark --all-combinations  # Все возможные комбинации
  poetry run benchmark --all-combinations --max-chain-length 3  # Ограничить длину цепочки
  poetry run benchmark --all-combinations --max-combinations-per-length 100  # Ограничить количество
        """
    )
    
    parser.add_argument(
        "--executable",
        type=str,
        default=str(default_executable),
        help=f"Путь к исполняемому файлу ImageFilter (по умолчанию: {default_executable})"
    )
    
    parser.add_argument(
        "--dataset",
        type=str,
        default="dataset",
        help="Директория с тестовыми изображениями (по умолчанию: dataset)"
    )
    
    parser.add_argument(
        "--output",
        type=str,
        default="benchmark_output",
        help="Директория для сохранения результатов (по умолчанию: benchmark_output)"
    )
    
    parser.add_argument(
        "--iterations",
        type=int,
        default=3,
        help="Количество итераций для каждого теста (по умолчанию: 3)"
    )
    
    parser.add_argument(
        "--pattern",
        type=str,
        default="*.jpg",
        help="Паттерн для поиска изображений (по умолчанию: *.jpg)"
    )
    
    parser.add_argument(
        "--chains",
        action="store_true",
        help="Запустить бенчмарк цепочек фильтров вместо одиночных фильтров"
    )
    
    parser.add_argument(
        "--both",
        action="store_true",
        help="Запустить бенчмарк как для одиночных фильтров, так и для цепочек"
    )
    
    parser.add_argument(
        "--all-combinations",
        action="store_true",
        help="Запустить бенчмарк для всех возможных комбинаций фильтров с учетом порядка"
    )
    
    parser.add_argument(
        "--max-chain-length",
        type=int,
        default=None,
        help="Максимальная длина цепочки для --all-combinations (по умолчанию: все фильтры)"
    )
    
    parser.add_argument(
        "--max-combinations-per-length",
        type=int,
        default=None,
        help="Максимальное количество комбинаций для каждой длины цепочки (для ограничения количества тестов)"
    )
    
    args = parser.parse_args()
    
    try:
        benchmark = ImageFilterBenchmark(
            executable_path=args.executable,
            dataset_dir=args.dataset,
            output_dir=args.output
        )
        
        if args.all_combinations:
            # Бенчмарк всех возможных комбинаций фильтров
            print("=" * 80)
            print("БЕНЧМАРК ВСЕХ ВОЗМОЖНЫХ КОМБИНАЦИЙ ФИЛЬТРОВ")
            print("=" * 80)
            benchmark.run_all_combinations_benchmark(
                iterations=args.iterations,
                image_pattern=args.pattern,
                max_chain_length=args.max_chain_length,
                max_combinations_per_length=args.max_combinations_per_length
            )
        elif args.chains:
            # Только бенчмарк цепочек
            benchmark.run_chain_benchmark(
                iterations=args.iterations,
                image_pattern=args.pattern
            )
        elif args.both:
            # Сначала одиночные фильтры, потом цепочки
            print("=" * 80)
            print("БЕНЧМАРК ОДИНОЧНЫХ ФИЛЬТРОВ")
            print("=" * 80)
            benchmark.run_benchmark(
                iterations=args.iterations,
                image_pattern=args.pattern
            )
            
            print("\n" + "=" * 80)
            print("БЕНЧМАРК ЦЕПОЧЕК ФИЛЬТРОВ")
            print("=" * 80)
            benchmark.run_chain_benchmark(
                iterations=args.iterations,
                image_pattern=args.pattern
            )
        else:
            raise ValueError("Необходимо указать хотя бы один из режимов работы: --chains, --both, --all-combinations")
        
        benchmark.print_statistics()
        benchmark.save_statistics_csv()
        benchmark.save_results_csv()
        
    except Exception as e:
        print(f"Ошибка: {e}")
        return 1
    
    return 0


if __name__ == "__main__":
    exit(main())

