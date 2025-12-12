#!/usr/bin/env python3
"""
Скрипт для генерации тестовых изображений разных размеров для бенчмарка.

Создает набор изображений различных размеров для тестирования производительности фильтров.
"""

import os
import numpy as np
from PIL import Image
from pathlib import Path


def generate_test_image(width: int, height: int, pattern: str = "gradient") -> Image.Image:
    """
    Генерирует тестовое изображение заданного размера.
    
    Args:
        width: Ширина изображения в пикселях
        height: Высота изображения в пикселях
        pattern: Тип паттерна ('gradient', 'checkerboard', 'noise', 'colorful')
    
    Returns:
        PIL Image объект
    """
    if pattern == "gradient":
        # Горизонтальный градиент
        img_array = np.zeros((height, width, 3), dtype=np.uint8)
        for x in range(width):
            intensity = int(255 * (x / width))
            img_array[:, x] = [intensity, intensity // 2, 255 - intensity]
    
    elif pattern == "checkerboard":
        # Шахматная доска
        img_array = np.zeros((height, width, 3), dtype=np.uint8)
        square_size = max(10, min(width, height) // 20)
        for y in range(height):
            for x in range(width):
                square_x = x // square_size
                square_y = y // square_size
                if (square_x + square_y) % 2 == 0:
                    img_array[y, x] = [255, 255, 255]
                else:
                    img_array[y, x] = [0, 0, 0]
    
    elif pattern == "noise":
        # Случайный шум
        img_array = np.random.randint(0, 256, (height, width, 3), dtype=np.uint8)
    
    elif pattern == "colorful":
        # Яркое цветное изображение с плавными переходами
        img_array = np.zeros((height, width, 3), dtype=np.uint8)
        for y in range(height):
            for x in range(width):
                r = int(128 + 127 * np.sin(x * 0.1))
                g = int(128 + 127 * np.sin(y * 0.1))
                b = int(128 + 127 * np.sin((x + y) * 0.1))
                img_array[y, x] = [r, g, b]
    
    else:
        raise ValueError(f"Неизвестный паттерн: {pattern}")
    
    return Image.fromarray(img_array)


def main():
    """Генерирует набор тестовых изображений для бенчмарка."""
    # Создаем директорию для датасета относительно подпроекта
    project_dir = Path(__file__).parent.parent
    dataset_dir = project_dir / "dataset"
    dataset_dir.mkdir(exist_ok=True)
    
    # Определяем размеры изображений для тестирования
    # Маленькие, средние, большие и очень большие
    test_sizes = [
        (256, 256, "small"),
        (512, 512, "medium"),
        (1024, 1024, "large"),
        (2048, 2048, "xlarge"),
        (3840, 2160, "4k"),  # 4K разрешение
        (256, 512, "portrait_small"),
        (512, 256, "landscape_small"),
        (1920, 1080, "fullhd"),
    ]
    
    patterns = ["gradient", "checkerboard", "colorful"]
    
    print("Генерация тестовых изображений...")
    generated_count = 0
    
    for width, height, size_name in test_sizes:
        for pattern in patterns:
            filename = dataset_dir / f"{size_name}_{pattern}_{width}x{height}.jpg"
            
            print(f"Создание: {filename.name} ({width}x{height})")
            img = generate_test_image(width, height, pattern)
            img.save(filename, quality=95)
            generated_count += 1
    
    print(f"\nГотово! Создано {generated_count} изображений в директории '{dataset_dir}'")


if __name__ == "__main__":
    main()

