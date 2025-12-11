#!/usr/bin/env python3
"""
Бенчмарк тест для измерения времени выполнения фильтров изображений.

Запускает C++ приложение ImageFilter для каждого фильтра и изображения,
измеряет время выполнения и выводит статистику.
"""

import os
import subprocess
import time
import json
import csv
import shutil
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
        
        # Удаляем выходную директорию, если она существует, и создаем заново
        if self.output_dir.exists():
            shutil.rmtree(self.output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
        if not self.executable_path.exists():
            raise FileNotFoundError(f"Исполняемый файл не найден: {executable_path}")
        
        if not self.dataset_dir.exists():
            raise FileNotFoundError(f"Директория с датасетом не найдена: {dataset_dir}")
    
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
            error_message=error_message
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
        
        # Общая статистика
        print(f"\nОбщая статистика:")
        print(f"  Всего тестов: {len(self.results)}")
        print(f"  Успешных: {len(successful_results)}")
        print(f"  Неудачных: {len(self.results) - len(successful_results)}")
        print(f"  Всего фильтров: {len(self.filters)}")
        for category_name, category_filters in self.filter_categories.items():
            category_success = len([r for r in successful_results if r.filter_name in category_filters])
            print(f"    {category_name}: {len(category_filters)} фильтров ({category_success} успешных)")
        
        # Группируем по категориям фильтров
        for category_name, category_filters in self.filter_categories.items():
            category_results = [r for r in successful_results if r.filter_name in category_filters]
            
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
                filter_times = [r.execution_time for r in results if r.filter_name == filter_name]
                if filter_times:
                    print(f"    {filter_name}: {mean(filter_times):.4f}s")
    
    def save_results_csv(self, filename: str = "benchmark_results.csv") -> None:
        """Сохраняет результаты в CSV файл."""
        csv_path = self.output_dir / filename
        
        with open(csv_path, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow([
                "Фильтр", "Изображение", "Ширина", "Высота", 
                "Пикселей", "Время (с)", "Успех", "Ошибка"
            ])
            
            for result in self.results:
                pixels = result.image_size[0] * result.image_size[1]
                writer.writerow([
                    result.filter_name,
                    Path(result.image_path).name,
                    result.image_size[0],
                    result.image_size[1],
                    pixels,
                    f"{result.execution_time:.6f}",
                    "Да" if result.success else "Нет",
                    result.error_message
                ])
        
        print(f"\nРезультаты сохранены в: {csv_path}")
    
    def save_results_json(self, filename: str = "benchmark_results.json") -> None:
        """Сохраняет результаты в JSON файл."""
        json_path = self.output_dir / filename
        
        data = {
            "results": [
                {
                    "filter": r.filter_name,
                    "image": Path(r.image_path).name,
                    "width": r.image_size[0],
                    "height": r.image_size[1],
                    "pixels": r.image_size[0] * r.image_size[1],
                    "time_seconds": r.execution_time,
                    "success": r.success,
                    "error": r.error_message
                }
                for r in self.results
            ]
        }
        
        with open(json_path, 'w', encoding='utf-8') as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
        
        print(f"Результаты сохранены в: {json_path}")
    
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
        
        csv_path = self.output_dir / filename
        
        with open(csv_path, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow([
                "Фильтр",
                "Количество тестов",
                "Среднее время (с)",
                "Медианное время (с)",
                "Стандартное отклонение (с)",
                "Минимальное время (с)",
                "Максимальное время (с)",
                "Производительность (MP/s)"
            ])
            
            # Группируем результаты по фильтрам
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
                
                writer.writerow([
                    filter_name,
                    count,
                    f"{avg_time:.4f}",
                    f"{median_time:.4f}",
                    f"{std_dev:.4f}",
                    f"{min_time:.4f}",
                    f"{max_time:.4f}",
                    f"{mp_per_sec:.2f}"
                ])
        
        print(f"Статистика сохранена в: {csv_path}")


def main():
    """Главная функция для запуска бенчмарка."""
    import argparse
    
    # Определяем путь к исполняемому файлу относительно подпроекта
    project_root = Path(__file__).parent.parent.parent
    default_executable = project_root / "bin" / "ImageFilter.exe"
    
    parser = argparse.ArgumentParser(
        description="Бенчмарк тест для фильтров изображений",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Примеры использования:
  poetry run benchmark
  poetry run benchmark --iterations 5
  poetry run benchmark --executable ../cmake-build-debug/bin/ImageFilter
  poetry run benchmark --dataset dataset --output benchmark_output
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
    
    args = parser.parse_args()
    
    try:
        benchmark = ImageFilterBenchmark(
            executable_path=args.executable,
            dataset_dir=args.dataset,
            output_dir=args.output
        )
        
        benchmark.run_benchmark(
            iterations=args.iterations,
            image_pattern=args.pattern
        )
        
        benchmark.print_statistics()
        benchmark.save_statistics_csv()
        benchmark.save_results_csv()
        benchmark.save_results_json()
        
    except Exception as e:
        print(f"Ошибка: {e}")
        return 1
    
    return 0


if __name__ == "__main__":
    exit(main())

