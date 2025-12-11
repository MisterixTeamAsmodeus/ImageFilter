#include <ImageProcessor.h>
#include <filters/IFilter.h>

#include <CLI/CLI.hpp>
#include <utils/Logger.h>
#include <utils/FilterFactory.h>
#include <utils/BatchProcessor.h>
#include <utils/Config.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <sstream>
#include <iomanip>


/**
 * @brief Выводит список всех доступных фильтров
 */
static void printFilterList()
{
    auto& factory = FilterFactory::getInstance();
    auto filter_names = factory.getRegisteredFilters();
    
    // Группируем фильтры по категориям
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> categories;
    
    CLI::App temp_app;
    for (const auto& name : filter_names)
    {
        auto filter = factory.create(name, temp_app);
        if (filter)
        {
            categories[filter->getCategory()].emplace_back(name, filter->getDescription());
        }
    }
    
    std::cout << "Доступные фильтры:" << std::endl;
    std::cout << std::endl;
    
    // Выводим фильтры по категориям
    for (const auto& [category, filters] : categories)
    {
        std::cout << category << ":" << std::endl;
        for (const auto& [name, description] : filters)
        {
            std::cout << "  " << std::setw(12) << std::left << name << " - " << description << std::endl;
        }
        std::cout << std::endl;
    }
}

/**
 * @brief Выводит информацию о конкретном фильтре
 */
static void printFilterInfo(const std::string& filter_name)
{
    auto& factory = FilterFactory::getInstance();
    
    if (!factory.isRegistered(filter_name))
    {
        std::cout << "Неизвестный фильтр: " << filter_name << std::endl;
        std::cout << "Используйте --list-filters для списка доступных фильтров" << std::endl;
        return;
    }
    
    CLI::App temp_app;
    auto filter = factory.create(filter_name, temp_app);
    if (!filter)
    {
        std::cout << "Ошибка: не удалось создать фильтр " << filter_name << std::endl;
        return;
    }
    
    std::cout << "Фильтр: " << filter->getName() << std::endl;
    std::cout << "Описание: " << filter->getDescription() << std::endl;
    std::cout << "Категория: " << filter->getCategory() << std::endl;
}

/**
 * @brief Разбивает строку фильтров на отдельные имена
 */
static std::vector<std::string> parseFilterChain(const std::string& filter_chain)
{
    std::vector<std::string> filters;
    std::stringstream ss(filter_chain);
    std::string filter;
    
    while (std::getline(ss, filter, ','))
    {
        // Убираем пробелы
        filter.erase(0, filter.find_first_not_of(" \t"));
        filter.erase(filter.find_last_not_of(" \t") + 1);
        
        if (!filter.empty())
        {
            filters.push_back(filter);
        }
    }
    
    return filters;
}

/**
 * @brief Обрабатывает одно изображение с применением фильтров
 * @param input_file Путь к входному файлу
 * @param output_file Путь к выходному файлу
 * @param filter_names Список имен фильтров для применения
 * @param app CLI::App для доступа к параметрам фильтров
 * @param preserve_alpha Сохранять ли альфа-канал
 * @param force_rgb Принудительно преобразовать RGBA в RGB
 * @param jpeg_quality Качество сохранения JPEG (0-100)
 * @return true если обработка успешна, false в противном случае
 */
static bool processSingleImage(const std::string& input_file,
                        const std::string& output_file,
                        const std::vector<std::string>& filter_names,
                        CLI::App& app,
                        bool preserve_alpha,
                        bool force_rgb,
                        int jpeg_quality)
{
    ImageProcessor image;
    
    // Устанавливаем качество JPEG
    if (!image.setJpegQuality(jpeg_quality))
    {
        Logger::error("Ошибка: недопустимое качество JPEG");
        return false;
    }
    
    if (!image.loadFromFile(input_file, preserve_alpha))
    {
        return false;
    }
    
    // Принудительное преобразование RGBA в RGB, если запрошено
    if (force_rgb && image.hasAlpha())
    {
        if (!image.convertToRGB())
        {
            return false;
        }
    }
    
    // Применяем фильтры по очереди
    auto& factory = FilterFactory::getInstance();
    for (const auto& filter_name : filter_names)
    {
        auto filter = factory.create(filter_name, app);
        if (!filter)
        {
            return false;
        }
        
        const auto result = filter->apply(image);
        if (!result.isSuccess())
        {
            return false;
        }
    }
    
    // Определяем, нужно ли сохранять альфа-канал
    bool save_alpha = preserve_alpha && image.hasAlpha() && !force_rgb;
    
    return image.saveToFile(output_file, save_alpha);
}

/**
 * @brief Отображает прогресс обработки файла
 * @param current Номер текущего файла
 * @param total Общее количество файлов
 * @param current_file Путь к текущему файлу
 */
static void displayProgress(size_t current, size_t total, const std::string& current_file)
{
    if (Logger::isQuiet())
    {
        return;
    }
    
    double percentage = (static_cast<double>(current) / static_cast<double>(total)) * 100.0;
    
    std::cout << "\r[" << std::setw(3) << std::fixed << std::setprecision(0) 
              << percentage << "%] "
              << "[" << current << "/" << total << "] "
              << "Обработка: " << current_file;
    
    if (current == total)
    {
        std::cout << std::endl;
    }
    else
    {
        std::cout.flush();
    }
}

int main(int argc, char* argv[])
{
    // Регистрируем все фильтры в фабрике
    FilterFactory::getInstance().registerAll();
    
    CLI::App app{"ImageFilter - Приложение для обработки JPEG-изображений", "ImageFilter"};
    
    // Основные параметры
    std::string input_file;
    std::string filter_name;
    std::string output_file;
    bool list_filters = false;
    std::string filter_info;
    bool quiet = false;
    std::string log_level_str = "INFO";
    bool preserve_alpha = false;
    bool force_rgb = false;
    int jpeg_quality = 90;
    
    // Параметры пакетной обработки
    bool batch_mode = false;
    std::string input_dir;
    std::string output_dir;
    bool recursive = false;
    std::string pattern;
    
    // Параметры фильтров
    double brightness_factor = 1.2;
    double contrast_factor = 1.5;
    double saturation_factor = 1.5;
    bool counter_clockwise = false;
    double blur_radius = 5.0;
    int box_blur_radius = 5;
    int motion_blur_length = 10;
    double motion_blur_angle = 0.0;
    int median_radius = 2;
    double noise_intensity = 0.1;
    int posterize_levels = 4;
    int threshold_value = 128;
    double vignette_strength = 0.5;
    
    // Опции командной строки
    app.add_option("input", input_file, "Входной файл изображения")->required(false);
    app.add_option("filter", filter_name, "Имя фильтра или цепочка фильтров через запятую")->required(false);
    app.add_option("output", output_file, "Выходной файл изображения")->required(false);
    
    app.add_flag("--list-filters", list_filters, "Вывести список доступных фильтров");
    app.add_option("--filter-info", filter_info, "Вывести информацию о конкретном фильтре");
    app.add_flag("-q,--quiet", quiet, "Тихий режим (минимальный вывод)");
    app.add_option("--log-level", log_level_str, "Уровень логирования (DEBUG, INFO, WARNING, ERROR, по умолчанию INFO)");
    app.add_flag("--preserve-alpha", preserve_alpha, "Сохранять альфа-канал при загрузке и сохранении (RGBA)");
    app.add_flag("--force-rgb", force_rgb, "Принудительно преобразовать RGBA в RGB перед обработкой");
    app.add_option("--jpeg-quality", jpeg_quality, "Качество сохранения JPEG изображений (0-100, по умолчанию 90)");
    
    // Опции для работы с пресетами
    std::string preset_file;
    std::string save_preset;
    std::string preset_dir;
    app.add_option("--preset", preset_file, "Загрузить пресет фильтров из файла");
    app.add_option("--save-preset", save_preset, "Сохранить текущую цепочку фильтров как пресет");
    app.add_option("--preset-dir", preset_dir, "Директория для сохранения/загрузки пресетов (по умолчанию: ./presets)");
    
    // Параметры пакетной обработки
    app.add_flag("--batch", batch_mode, "Включить пакетный режим обработки");
    app.add_option("--input-dir", input_dir, "Входная директория с изображениями (для пакетного режима)");
    app.add_option("--output-dir", output_dir, "Выходная директория для обработанных изображений (для пакетного режима)");
    app.add_flag("--recursive", recursive, "Рекурсивный обход поддиректорий (для пакетного режима)");
    app.add_option("--pattern", pattern, "Шаблон для фильтрации файлов (например, *.jpg, *.png)");
    
    // Параметры фильтров
    app.add_option("--brightness-factor", brightness_factor, "Коэффициент яркости (по умолчанию 1.2)");
    app.add_option("--contrast-factor", contrast_factor, "Коэффициент контрастности (по умолчанию 1.5)");
    app.add_option("--saturation-factor", saturation_factor, "Коэффициент насыщенности (по умолчанию 1.5)");
    app.add_flag("--counter-clockwise", counter_clockwise, "Поворот против часовой стрелки (для rotate90)");
    app.add_option("--blur-radius", blur_radius, "Радиус размытия по Гауссу (по умолчанию 5.0)");
    app.add_option("--box-blur-radius", box_blur_radius, "Радиус размытия по прямоугольнику (по умолчанию 5)");
    app.add_option("--motion-blur-length", motion_blur_length, "Длина размытия движения (по умолчанию 10)");
    app.add_option("--motion-blur-angle", motion_blur_angle, "Угол размытия движения в градусах (по умолчанию 0.0)");
    app.add_option("--median-radius", median_radius, "Радиус медианного фильтра (по умолчанию 2)");
    app.add_option("--noise-intensity", noise_intensity, "Интенсивность шума (по умолчанию 0.1, диапазон 0.0-1.0)");
    app.add_option("--posterize-levels", posterize_levels, "Количество уровней постеризации (по умолчанию 4, диапазон 2-256)");
    app.add_option("--threshold-value", threshold_value, "Пороговое значение бинаризации (по умолчанию 128, диапазон 0-255)");
    app.add_option("--vignette-strength", vignette_strength, "Сила виньетирования (по умолчанию 0.5, диапазон 0.0-1.0)");
    
    // Парсим аргументы
    try
    {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError& e)
    {
        return app.exit(e);
    }
    
    // Настраиваем логирование
    if (quiet)
    {
        Logger::setQuiet(true);
    }
    else
    {
        if (log_level_str == "DEBUG")
        {
            Logger::setLevel(LogLevel::DEBUG);
        }
        else if (log_level_str == "INFO")
        {
            Logger::setLevel(LogLevel::INFO);
        }
        else if (log_level_str == "WARNING")
        {
            Logger::setLevel(LogLevel::WARNING);
        }
        else if (log_level_str == "ERROR")
        {
            Logger::setLevel(LogLevel::ERROR);
        }
    }
    
    // Обработка специальных команд
    if (list_filters)
    {
        printFilterList();
        return 0;
    }
    
    if (!filter_info.empty())
    {
        printFilterInfo(filter_info);
        return 0;
    }
    
    // Обработка пресетов
    Config config;
    std::string preset_directory = preset_dir.empty() ? "./presets" : preset_dir;
    
    // Если указан пресет, загружаем его
    if (!preset_file.empty())
    {
        std::string preset_path = preset_file;
        if (!preset_file.empty() && preset_file[0] != '/' && preset_file[0] != '\\' && preset_file.find(':') == std::string::npos)
        {
            // Относительный путь - добавляем директорию пресетов
            preset_path = preset_directory + "/" + preset_file;
            if (preset_path.length() < 5 || preset_path.substr(preset_path.length() - 5) != ".json")
            {
                preset_path += ".json";
            }
        }
        
        auto preset = config.loadPreset(preset_path);
        if (!preset)
        {
            Logger::error("Ошибка: не удалось загрузить пресет: " + preset_path);
            return 1;
        }
        
        if (!config.validateConfig(*preset))
        {
            Logger::error("Ошибка: пресет содержит невалидную конфигурацию");
            return 1;
        }
        
        // Преобразуем пресет в цепочку фильтров
        filter_name.clear();
        for (size_t i = 0; i < preset->filters.size(); ++i)
        {
            if (i > 0)
            {
                filter_name += ",";
            }
            filter_name += preset->filters[i].name;
        }
        
        Logger::info("Загружен пресет: " + preset->name);
        if (!preset->description.empty())
        {
            Logger::info("Описание: " + preset->description);
        }
    }
    
    // Парсим цепочку фильтров (нужно для обоих режимов)
    auto filters = parseFilterChain(filter_name);
    if (filters.empty() && !batch_mode && save_preset.empty())
    {
        Logger::error("Ошибка: не указаны фильтры для применения");
        return 1;
    }
    
    // Сохранение пресета
    if (!save_preset.empty())
    {
        if (filters.empty())
        {
            Logger::error("Ошибка: не указаны фильтры для сохранения в пресет");
            return 1;
        }
        
        FilterChainConfig preset_config;
        preset_config.name = save_preset;
        preset_config.description = "Сохраненный пресет: " + filter_name;
        
        // Преобразуем цепочку фильтров в конфигурацию
        for (const auto& filter_name_str : filters)
        {
            FilterConfig filter_cfg;
            filter_cfg.name = filter_name_str;
            
            // Извлекаем параметры из CLI::App для каждого фильтра
            // Это упрощенная версия - в реальности нужно извлекать параметры для каждого фильтра
            // Здесь мы просто сохраняем имя фильтра, параметры можно добавить позже
            
            preset_config.filters.push_back(filter_cfg);
        }
        
        std::string preset_path = preset_directory + "/" + save_preset + ".json";
        if (config.savePreset(preset_path, preset_config))
        {
            Logger::info("Пресет сохранен: " + preset_path);
        }
        else
        {
            Logger::error("Ошибка: не удалось сохранить пресет: " + preset_path);
            return 1;
        }
        
        // Если только сохранение пресета, выходим
        if (input_file.empty() && output_file.empty())
        {
            return 0;
        }
    }
    
    // Пакетный режим обработки
    if (batch_mode)
    {
        if (input_dir.empty() || output_dir.empty() || filter_name.empty())
        {
            Logger::error("Ошибка: в пакетном режиме необходимо указать --input-dir, --output-dir и filter");
            Logger::error("Используйте --help для справки");
            return 1;
        }
        
        if (filters.empty())
        {
            Logger::error("Ошибка: не указаны фильтры для применения");
            return 1;
        }
        
        Logger::info("Пакетный режим обработки");
        Logger::info("Входная директория: " + input_dir);
        Logger::info("Выходная директория: " + output_dir);
        if (recursive)
        {
            Logger::info("Рекурсивный обход: включен");
        }
        if (!pattern.empty())
        {
            Logger::info("Шаблон фильтрации: " + pattern);
        }
        
        // Создаем процессор пакетной обработки
        BatchProcessor processor(input_dir, output_dir, recursive, pattern);
        
        // Функция обработки одного файла
        auto process_function = [&](const std::string& input_path, const std::string& output_path) -> bool {
            return processSingleImage(input_path, output_path, filters, app, preserve_alpha, force_rgb, jpeg_quality);
        };
        
        // Callback для отображения прогресса
        ProgressCallback progress_callback = displayProgress;
        
        // Обрабатываем все файлы
        BatchStatistics stats = processor.processAll(process_function, progress_callback);
        
        // Выводим статистику
        Logger::info("Пакетная обработка завершена:");
        Logger::info("  Всего файлов: " + std::to_string(stats.total_files));
        Logger::info("  Успешно обработано: " + std::to_string(stats.processed_files));
        Logger::info("  Ошибок: " + std::to_string(stats.failed_files));
        Logger::info("  Пропущено: " + std::to_string(stats.skipped_files));
        
        return (stats.failed_files > 0) ? 1 : 0;
    }
    
    // Обычный режим обработки одного файла
    if (input_file.empty() || filter_name.empty() || output_file.empty())
    {
        Logger::error("Ошибка: необходимо указать input, filter и output");
        Logger::error("Используйте --help для справки");
        Logger::error("Или --list-filters для списка доступных фильтров");
        return 1;
    }
    
    // Загружаем изображение
    ImageProcessor image;
    
    // Устанавливаем качество JPEG
    if (!image.setJpegQuality(jpeg_quality))
    {
        Logger::error("Ошибка: недопустимое качество JPEG");
        return 1;
    }
    
    Logger::info("Загрузка изображения: " + input_file);
    
    if (!image.loadFromFile(input_file, preserve_alpha))
    {
        Logger::error("Не удалось загрузить изображение: " + input_file);
        return 1;
    }
    
    Logger::info("Изображение загружено: " + std::to_string(image.getWidth()) + 
                 "x" + std::to_string(image.getHeight()) + " пикселей, " +
                 std::to_string(image.getChannels()) + " каналов");
    
    // Принудительное преобразование RGBA в RGB, если запрошено
    if (force_rgb && image.hasAlpha())
    {
        Logger::info("Преобразование RGBA в RGB...");
        if (!image.convertToRGB())
        {
            Logger::error("Не удалось преобразовать RGBA в RGB");
            return 1;
        }
        Logger::info("Изображение преобразовано в RGB");
    }
    
    // Применяем фильтры по очереди
    for (size_t i = 0; i < filters.size(); ++i)
    {
        const auto& current_filter_name = filters[i];
        
        Logger::info("Применение фильтра " + std::to_string(i + 1) + "/" + 
                     std::to_string(filters.size()) + ": " + current_filter_name);
        
        auto& factory = FilterFactory::getInstance();
        auto filter = factory.create(current_filter_name, app);
        if (!filter)
        {
            Logger::error("Неизвестный фильтр: " + current_filter_name);
            Logger::error("Используйте --list-filters для списка доступных фильтров");
            return 1;
        }
        
        const auto result = filter->apply(image);
        if (!result.isSuccess())
        {
            Logger::error("Не удалось применить фильтр: " + current_filter_name);
            if (!result.message.empty())
            {
                Logger::error("Ошибка: " + result.message);
            }
            return 1;
        }
    }
    
    // Сохраняем результат
    Logger::info("Сохранение результата: " + output_file);
    
    // Определяем, нужно ли сохранять альфа-канал
    // Если изображение имеет альфа-канал и не было преобразовано в RGB, сохраняем его
    bool save_alpha = preserve_alpha && image.hasAlpha() && !force_rgb;
    
    if (!image.saveToFile(output_file, save_alpha))
    {
        Logger::error("Не удалось сохранить изображение: " + output_file);
        return 1;
    }
    
    Logger::info("Готово! Результат сохранен в " + output_file);
    
    return 0;
}
