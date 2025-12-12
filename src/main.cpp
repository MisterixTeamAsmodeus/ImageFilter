#include <ImageProcessor.h>
#include <filters/IFilter.h>
#include <filters/GrayscaleFilter.h>
#include <filters/GaussianBlurFilter.h>
#include <filters/SharpenFilter.h>
#include <filters/SepiaFilter.h>
#include <filters/InvertFilter.h>
#include <filters/BrightnessFilter.h>
#include <filters/ContrastFilter.h>
#include <filters/SaturationFilter.h>
#include <filters/FlipHorizontalFilter.h>
#include <filters/FlipVerticalFilter.h>
#include <filters/Rotate90Filter.h>
#include <filters/EdgeDetectionFilter.h>
#include <filters/EmbossFilter.h>
#include <filters/OutlineFilter.h>
#include <filters/MotionBlurFilter.h>
#include <filters/MedianFilter.h>
#include <filters/NoiseFilter.h>
#include <filters/PosterizeFilter.h>
#include <filters/ThresholdFilter.h>
#include <filters/VignetteFilter.h>
#include <filters/BoxBlurFilter.h>

#include <iostream>
#include <string>
#include <map>
#include <memory>

/**
 * @brief Главная функция приложения
 * 
 * Приложение принимает аргументы командной строки:
 * ./ImageFilter <input.jpg> <filter_name> <output.jpg>
 * 
 * Доступные фильтры:
 * Цветовые:
 * - grayscale: Преобразование в оттенки серого
 * - sepia: Эффект сепии (винтажный)
 * - invert: Инверсия цветов
 * - brightness: Изменение яркости (по умолчанию 1.2)
 * - contrast: Изменение контрастности (по умолчанию 1.5)
 * - saturation: Изменение насыщенности (по умолчанию 1.5)
 * 
 * Геометрические:
 * - flip_h: Горизонтальное отражение
 * - flip_v: Вертикальное отражение
 * - rotate90: Поворот на 90 градусов (по часовой стрелке)
 * 
 * Края и детали:
 * - sharpen: Повышение резкости
 * - edges: Детекция краёв (оператор Собеля)
 * - emboss: Эффект рельефа
 * - outline: Выделение контуров
 * 
 * Размытие и шум:
 * - blur: Размытие по Гауссу (радиус 5.0)
 * - box_blur: Размытие по прямоугольнику (радиус 5)
 * - motion_blur: Размытие движения (длина 10, угол 0°)
 * - median: Медианный фильтр (удаление шума, радиус 2)
 * - noise: Добавление шума (интенсивность 0.1)
 * 
 * Стилистические:
 * - posterize: Постеризация (4 уровня)
 * - threshold: Пороговая бинаризация (порог 128)
 * - vignette: Виньетирование (сила 0.5)
 */
int main(int argc, char* argv[]) {
    // Проверяем количество аргументов командной строки
    if (argc != 4) {
        std::cerr << "Использование: " << argv[0] << " <input.jpg> <filter_name> <output.jpg>" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Доступные фильтры:" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Цветовые:" << std::endl;
        std::cerr << "  grayscale  - Преобразование в оттенки серого" << std::endl;
        std::cerr << "  sepia      - Эффект сепии (винтажный)" << std::endl;
        std::cerr << "  invert     - Инверсия цветов" << std::endl;
        std::cerr << "  brightness - Изменение яркости" << std::endl;
        std::cerr << "  contrast   - Изменение контрастности" << std::endl;
        std::cerr << "  saturation - Изменение насыщенности" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Геометрические:" << std::endl;
        std::cerr << "  flip_h     - Горизонтальное отражение" << std::endl;
        std::cerr << "  flip_v     - Вертикальное отражение" << std::endl;
        std::cerr << "  rotate90   - Поворот на 90 градусов" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Края и детали:" << std::endl;
        std::cerr << "  sharpen    - Повышение резкости" << std::endl;
        std::cerr << "  edges      - Детекция краёв" << std::endl;
        std::cerr << "  emboss     - Эффект рельефа" << std::endl;
        std::cerr << "  outline    - Выделение контуров" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Размытие и шум:" << std::endl;
        std::cerr << "  blur       - Размытие по Гауссу" << std::endl;
        std::cerr << "  box_blur   - Размытие по прямоугольнику" << std::endl;
        std::cerr << "  motion_blur - Размытие движения" << std::endl;
        std::cerr << "  median     - Медианный фильтр" << std::endl;
        std::cerr << "  noise      - Добавление шума" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Стилистические:" << std::endl;
        std::cerr << "  posterize  - Постеризация" << std::endl;
        std::cerr << "  threshold  - Пороговая бинаризация" << std::endl;
        std::cerr << "  vignette   - Виньетирование" << std::endl;
        return 1;
    }

    const auto input_file = std::string(argv[1]);
    const auto filter_name = std::string(argv[2]);
    const auto output_file = std::string(argv[3]);

    // Создаем объект для работы с изображением
    ImageProcessor image;

    // Загружаем изображение
    std::cout << "Загрузка изображения: " << input_file << std::endl;
    if (!image.loadFromFile(input_file)) {
        std::cerr << "Ошибка: не удалось загрузить изображение" << std::endl;
        return 1;
    }

    std::cout << "Изображение загружено: " << image.getWidth() 
              << "x" << image.getHeight() << " пикселей" << std::endl;

    std::map<std::string, std::unique_ptr<IFilter>> filters;
    
    // Цветовые фильтры
    filters.insert(std::make_pair("grayscale", std::make_unique<GrayscaleFilter>()));
    filters.insert(std::make_pair("sepia", std::make_unique<SepiaFilter>()));
    filters.insert(std::make_pair("invert", std::make_unique<InvertFilter>()));
    filters.insert(std::make_pair("brightness", std::make_unique<BrightnessFilter>(1.2)));
    filters.insert(std::make_pair("contrast", std::make_unique<ContrastFilter>(1.5)));
    filters.insert(std::make_pair("saturation", std::make_unique<SaturationFilter>(1.5)));
    
    // Геометрические фильтры
    filters.insert(std::make_pair("flip_h", std::make_unique<FlipHorizontalFilter>()));
    filters.insert(std::make_pair("flip_v", std::make_unique<FlipVerticalFilter>()));
    filters.insert(std::make_pair("rotate90", std::make_unique<Rotate90Filter>(true)));
    
    // Фильтры краёв и деталей
    filters.insert(std::make_pair("sharpen", std::make_unique<SharpenFilter>()));
    filters.insert(std::make_pair("edges", std::make_unique<EdgeDetectionFilter>()));
    filters.insert(std::make_pair("emboss", std::make_unique<EmbossFilter>()));
    filters.insert(std::make_pair("outline", std::make_unique<OutlineFilter>()));
    
    // Фильтры размытия и шума
    filters.insert(std::make_pair("blur", std::make_unique<GaussianBlurFilter>(5.0)));
    filters.insert(std::make_pair("box_blur", std::make_unique<BoxBlurFilter>(5)));
    filters.insert(std::make_pair("motion_blur", std::make_unique<MotionBlurFilter>(10, 0.0)));
    filters.insert(std::make_pair("median", std::make_unique<MedianFilter>(2)));
    filters.insert(std::make_pair("noise", std::make_unique<NoiseFilter>(0.1)));
    
    // Стилистические фильтры
    filters.insert(std::make_pair("posterize", std::make_unique<PosterizeFilter>(4)));
    filters.insert(std::make_pair("threshold", std::make_unique<ThresholdFilter>(128)));
    filters.insert(std::make_pair("vignette", std::make_unique<VignetteFilter>(0.5)));

    // Ищем выбранный фильтр
    auto filter_it = filters.find(filter_name);
    if (filter_it == filters.end()) {
        std::cerr << "Ошибка: неизвестный фильтр '" << filter_name << "'" << std::endl;
        std::cerr << "Используйте " << argv[0] << " без параметров для списка доступных фильтров" << std::endl;
        return 1;
    }

    // Применяем фильтр через указатель на интерфейс
    std::cout << "Применение фильтра: " << filter_name << std::endl;
    if (!filter_it->second->apply(image)) {
        std::cerr << "Ошибка: не удалось применить фильтр" << std::endl;
        return 1;
    }

    // Сохраняем результат
    std::cout << "Сохранение результата: " << output_file << std::endl;
    if (!image.saveToFile(output_file)) {
        std::cerr << "Ошибка: не удалось сохранить изображение" << std::endl;
        return 1;
    }

    std::cout << "Готово! Результат сохранен в " << output_file << std::endl;
    return 0;
}

