#pragma once

#include <utils/FilterResult.h>
#include <cstdint>

class ImageProcessor;

/**
 * @brief Валидатор целостности данных изображения
 * 
 * Предоставляет функции для проверки целостности данных изображения,
 * включая проверку на поврежденные данные, некорректные значения пикселей
 * и другие проблемы с данными.
 */
namespace ImageValidator
{
    /**
     * @brief Проверяет базовую валидность изображения
     * 
     * Проверяет, что изображение загружено и имеет корректные размеры.
     * 
     * @param image Изображение для проверки
     * @return FilterResult с ошибкой, если изображение невалидно, иначе Success
     */
    FilterResult validateBasic(const ImageProcessor& image);

    /**
     * @brief Проверяет целостность данных изображения
     * 
     * Выполняет глубокую проверку данных изображения:
     * - Проверка на nullptr указатель
     * - Проверка корректности размеров
     * - Проверка на поврежденные данные (выборочная проверка)
     * 
     * @param image Изображение для проверки
     * @param deep_check Если true, выполняет глубокую проверку всех пикселей (медленнее)
     * @return FilterResult с ошибкой, если данные повреждены, иначе Success
     */
    FilterResult validateDataIntegrity(const ImageProcessor& image, bool deep_check = false);

    /**
     * @brief Проверяет, что указатель на данные не является nullptr
     * 
     * @param data Указатель на данные изображения
     * @param width Ширина изображения
     * @param height Высота изображения
     * @param channels Количество каналов
     * @return FilterResult с ошибкой, если указатель nullptr, иначе Success
     */
    FilterResult validateDataPointer(const uint8_t* data, int width, int height, int channels);

    /**
     * @brief Проверяет корректность значений пикселей
     * 
     * Проверяет, что все значения пикселей находятся в допустимом диапазоне [0, 255].
     * Выполняет выборочную проверку для производительности.
     * 
     * @param image Изображение для проверки
     * @param sample_rate Частота выборки (1.0 = проверка всех пикселей, 0.1 = 10% пикселей)
     * @return FilterResult с ошибкой, если найдены некорректные значения, иначе Success
     */
    FilterResult validatePixelValues(const ImageProcessor& image, double sample_rate = 0.1);

    /**
     * @brief Проверяет, что размер данных соответствует заявленным размерам
     * 
     * @param image Изображение для проверки
     * @return FilterResult с ошибкой, если размеры не соответствуют, иначе Success
     */
    FilterResult validateDataSize(const ImageProcessor& image);

    /**
     * @brief Проверяет, что изображение не пустое
     * 
     * @param image Изображение для проверки
     * @return FilterResult с ошибкой, если изображение пустое, иначе Success
     */
    FilterResult validateNotEmpty(const ImageProcessor& image);

    /**
     * @brief Выполняет полную валидацию изображения
     * 
     * Выполняет все проверки: базовую валидность, целостность данных,
     * корректность значений пикселей и размеров.
     * 
     * @param image Изображение для проверки
     * @param deep_check Если true, выполняет глубокую проверку всех пикселей
     * @return FilterResult с ошибкой, если какая-либо проверка не прошла, иначе Success
     */
    FilterResult validateFull(const ImageProcessor& image, bool deep_check = false);
}

