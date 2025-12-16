#pragma once

#include <string>
#include <vector>
#include <CLI/CLI.hpp>

// Forward declaration
class ImageProcessor;

/**
 * @brief Вспомогательный класс для обработки изображений
 * 
 * Отвечает за:
 * - Обработку одного изображения с применением цепочки фильтров
 * - Управление пулом буферов
 * - Преобразование форматов изображений
 */
class ImageProcessingHelper
{
public:
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
    static bool processSingleImage(
        const std::string& input_file,
        const std::string& output_file,
        const std::vector<std::string>& filter_names,
        CLI::App& app,
        bool preserve_alpha,
        bool force_rgb,
        int jpeg_quality);

    /**
     * @brief Разбивает строку фильтров на отдельные имена
     * @param filter_chain Строка с фильтрами через запятую
     * @return Вектор имен фильтров
     */
    static std::vector<std::string> parseFilterChain(const std::string& filter_chain);
};

