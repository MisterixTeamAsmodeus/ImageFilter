#include <cli/CommandParser.h>
#include <utils/Logger.h>

CommandParser::CommandParser()
    : app_("ImageFilter - Приложение для обработки изображений", "ImageFilter")
{
}

int CommandParser::parse(int argc, char* argv[], CommandOptions& options)
{
    setupOptions(options);
    
    try
    {
        app_.parse(argc, argv);
        return 0;
    }
    catch (const CLI::ParseError& e)
    {
        return app_.exit(e);
    }
}

CLI::App& CommandParser::getApp() noexcept
{
    return app_;
}

void CommandParser::setupOptions(CommandOptions& options)
{
    // Основные параметры
    app_.add_option("input", options.input_file, "Входной файл изображения")->required(false);
    app_.add_option("filter", options.filter_name, "Имя фильтра или цепочка фильтров через запятую")->required(false);
    app_.add_option("output", options.output_file, "Выходной файл изображения")->required(false);
    
    app_.add_flag("--list-filters", options.list_filters, "Вывести список доступных фильтров");
    app_.add_option("--filter-info", options.filter_info, "Вывести информацию о конкретном фильтре");
    app_.add_flag("-q,--quiet", options.quiet, "Тихий режим (минимальный вывод)");
    app_.add_option("--log-level", options.log_level_str, "Уровень логирования (DEBUG, INFO, WARNING, ERROR, по умолчанию INFO)");
    app_.add_flag("--preserve-alpha", options.preserve_alpha, "Сохранять альфа-канал при загрузке и сохранении (RGBA)");
    app_.add_flag("--force-rgb", options.force_rgb, "Принудительно преобразовать RGBA в RGB перед обработкой");
    app_.add_option("--jpeg-quality", options.jpeg_quality, "Качество сохранения JPEG изображений (0-100, по умолчанию 90)");
    
    // Опции для работы с пресетами
    app_.add_option("--preset", options.preset_file, "Загрузить пресет фильтров из файла");
    app_.add_option("--save-preset", options.save_preset, "Сохранить текущую цепочку фильтров как пресет");
    app_.add_option("--preset-dir", options.preset_dir, "Директория для сохранения/загрузки пресетов (по умолчанию: ./presets)");
    
    // Параметры пакетной обработки
    app_.add_flag("--batch", options.batch_mode, "Включить пакетный режим обработки");
    app_.add_option("--input-dir", options.input_dir, "Входная директория с изображениями (для пакетного режима)");
    app_.add_option("--output-dir", options.output_dir, "Выходная директория для обработанных изображений (для пакетного режима)");
    app_.add_flag("--recursive", options.recursive, "Рекурсивный обход поддиректорий (для пакетного режима)");
    app_.add_option("--pattern", options.pattern, "Шаблон для фильтрации файлов (например, *.jpg, *.png)");
    app_.add_option("--resume-state", options.resume_state_file, "Файл для сохранения/загрузки состояния возобновления пакетной обработки");
    
    // Параметры фильтров
    app_.add_option("--brightness-factor", options.brightness_factor, "Коэффициент яркости (по умолчанию 1.2)");
    app_.add_option("--contrast-factor", options.contrast_factor, "Коэффициент контрастности (по умолчанию 1.5)");
    app_.add_option("--saturation-factor", options.saturation_factor, "Коэффициент насыщенности (по умолчанию 1.5)");
    app_.add_flag("--counter-clockwise", options.counter_clockwise, "Поворот против часовой стрелки (для rotate90)");
    app_.add_option("--blur-radius", options.blur_radius, "Радиус размытия по Гауссу (по умолчанию 5.0)");
    app_.add_option("--box-blur-radius", options.box_blur_radius, "Радиус размытия по прямоугольнику (по умолчанию 5)");
    app_.add_option("--motion-blur-length", options.motion_blur_length, "Длина размытия движения (по умолчанию 10)");
    app_.add_option("--motion-blur-angle", options.motion_blur_angle, "Угол размытия движения в градусах (по умолчанию 0.0)");
    app_.add_option("--median-radius", options.median_radius, "Радиус медианного фильтра (по умолчанию 2)");
    app_.add_option("--noise-intensity", options.noise_intensity, "Интенсивность шума (по умолчанию 0.1, диапазон 0.0-1.0)");
    app_.add_option("--posterize-levels", options.posterize_levels, "Количество уровней постеризации (по умолчанию 4, диапазон 2-256)");
    app_.add_option("--threshold-value", options.threshold_value, "Пороговое значение бинаризации (по умолчанию 128, диапазон 0-255)");
    app_.add_option("--vignette-strength", options.vignette_strength, "Сила виньетирования (по умолчанию 0.5, диапазон 0.0-1.0)");
    app_.add_option("--sharpen-strength", options.sharpen_strength, "Сила эффекта резкости (по умолчанию 1.0, >= 0.0)");
    app_.add_option("--edge-sensitivity", options.edge_sensitivity, "Чувствительность детекции краёв (по умолчанию 0.5, диапазон 0.0-1.0)");
    app_.add_option("--edge-operator", options.edge_operator, "Оператор детекции краёв: sobel, prewitt, scharr (по умолчанию sobel)");
    app_.add_option("--emboss-strength", options.emboss_strength, "Сила эффекта рельефа (по умолчанию 1.0, >= 0.0)");
}

