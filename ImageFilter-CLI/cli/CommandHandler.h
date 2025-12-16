#pragma once

#include <string>

/**
 * @brief Структура для хранения распарсенных параметров командной строки
 */
struct CommandOptions
{
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
    std::string resume_state_file;  // Файл для сохранения/загрузки состояния возобновления
    
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
    double sharpen_strength = 1.0;
    double edge_sensitivity = 0.5;
    std::string edge_operator = "sobel";  // sobel, prewitt, scharr
    double emboss_strength = 1.0;
    
    // Параметры пресетов
    std::string preset_file;
    std::string save_preset;
    std::string preset_dir;
};


