#pragma once

#include <utils/FilterResult.h>
#include <cstdint>
#include <limits>
#include <string>

/**
 * @brief Валидатор параметров фильтров
 * 
 * Предоставляет централизованные функции для валидации параметров фильтров
 * с защитой от переполнения и проверкой допустимых диапазонов значений.
 */
namespace FilterValidator
{
    /**
     * @brief Валидирует радиус фильтра
     * 
     * @param radius Радиус для проверки
     * @param min_radius Минимальное допустимое значение радиуса (по умолчанию 0)
     * @param max_radius Максимальное допустимое значение радиуса (по умолчанию 1000)
     * @param width Ширина изображения (для проверки, что радиус не превышает размер)
     * @param height Высота изображения (для проверки, что радиус не превышает размер)
     * @return FilterResult с ошибкой, если валидация не прошла, иначе Success
     */
    FilterResult validateRadius(int radius, int min_radius = 0, int max_radius = 1000,
                                int width = 0, int height = 0);

    /**
     * @brief Валидирует радиус фильтра (double)
     * 
     * @param radius Радиус для проверки
     * @param min_radius Минимальное допустимое значение радиуса (по умолчанию 0.0)
     * @param max_radius Максимальное допустимое значение радиуса (по умолчанию 1000.0)
     * @param width Ширина изображения (для проверки, что радиус не превышает размер)
     * @param height Высота изображения (для проверки, что радиус не превышает размер)
     * @return FilterResult с ошибкой, если валидация не прошла, иначе Success
     */
    FilterResult validateRadius(double radius, double min_radius = 0.0, double max_radius = 1000.0,
                                int width = 0, int height = 0);

    /**
     * @brief Валидирует коэффициент (factor)
     * 
     * @param factor Коэффициент для проверки
     * @param min_factor Минимальное допустимое значение (по умолчанию 0.0)
     * @param max_factor Максимальное допустимое значение (по умолчанию 10.0)
     * @return FilterResult с ошибкой, если валидация не прошла, иначе Success
     */
    FilterResult validateFactor(double factor, double min_factor = 0.0, double max_factor = 10.0);

    /**
     * @brief Валидирует пороговое значение
     * 
     * @param threshold Пороговое значение (обычно в диапазоне 0-255)
     * @param min_threshold Минимальное допустимое значение (по умолчанию 0)
     * @param max_threshold Максимальное допустимое значение (по умолчанию 255)
     * @return FilterResult с ошибкой, если валидация не прошла, иначе Success
     */
    FilterResult validateThreshold(int threshold, int min_threshold = 0, int max_threshold = 255);

    /**
     * @brief Валидирует интенсивность (обычно в диапазоне 0.0-1.0)
     * 
     * @param intensity Интенсивность для проверки
     * @param min_intensity Минимальное допустимое значение (по умолчанию 0.0)
     * @param max_intensity Максимальное допустимое значение (по умолчанию 1.0)
     * @return FilterResult с ошибкой, если валидация не прошла, иначе Success
     */
    FilterResult validateIntensity(double intensity, double min_intensity = 0.0, double max_intensity = 1.0);

    /**
     * @brief Валидирует качество (обычно в диапазоне 0-100)
     * 
     * @param quality Качество для проверки
     * @param min_quality Минимальное допустимое значение (по умолчанию 0)
     * @param max_quality Максимальное допустимое значение (по умолчанию 100)
     * @return FilterResult с ошибкой, если валидация не прошла, иначе Success
     */
    FilterResult validateQuality(int quality, int min_quality = 0, int max_quality = 100);

    /**
     * @brief Валидирует угол поворота
     * 
     * @param angle Угол в градусах для проверки
     * @param min_angle Минимальное допустимое значение (по умолчанию -360)
     * @param max_angle Максимальное допустимое значение (по умолчанию 360)
     * @return FilterResult с ошибкой, если валидация не прошла, иначе Success
     */
    FilterResult validateAngle(double angle, double min_angle = -360.0, double max_angle = 360.0);

    /**
     * @brief Валидирует размер ядра свертки
     * 
     * @param kernel_size Размер ядра (должен быть нечетным и положительным)
     * @param min_size Минимальный размер (по умолчанию 1)
     * @param max_size Максимальный размер (по умолчанию 100)
     * @return FilterResult с ошибкой, если валидация не прошла, иначе Success
     */
    FilterResult validateKernelSize(int kernel_size, int min_size = 1, int max_size = 100);

    /**
     * @brief Валидирует размер изображения с проверкой на переполнение
     * 
     * @param width Ширина изображения
     * @param height Высота изображения
     * @param channels Количество каналов
     * @return FilterResult с ошибкой, если валидация не прошла, иначе Success
     */
    FilterResult validateImageSize(int width, int height, int channels);

    /**
     * @brief Валидирует, что вычисление размера буфера не приведет к переполнению
     * 
     * @param width Ширина изображения
     * @param height Высота изображения
     * @param channels Количество каналов
     * @param buffer_size Результат вычисления размера буфера (выходной параметр)
     * @return FilterResult с ошибкой, если вычисление привело к переполнению, иначе Success
     */
    FilterResult validateBufferSize(int width, int height, int channels, size_t& buffer_size);

    /**
     * @brief Валидирует параметр с проверкой диапазона
     * 
     * @tparam T Тип параметра (int, double, float)
     * @param value Значение для проверки
     * @param min_value Минимальное допустимое значение
     * @param max_value Максимальное допустимое значение
     * @param param_name Имя параметра для сообщения об ошибке
     * @return FilterResult с ошибкой, если валидация не прошла, иначе Success
     */
    template<typename T>
    FilterResult validateRange(T value, T min_value, T max_value, const std::string& param_name)
    {
        if (value < min_value || value > max_value)
        {
            ErrorContext ctx;
            ctx.filter_params = param_name + "=" + std::to_string(value) + 
                               " (допустимый диапазон: [" + std::to_string(min_value) + 
                               ", " + std::to_string(max_value) + "])";
            return FilterResult::failure(FilterError::ParameterOutOfRange,
                                       "Параметр '" + param_name + "' вне допустимого диапазона: " +
                                       std::to_string(value) + " (допустимо: [" +
                                       std::to_string(min_value) + ", " + 
                                       std::to_string(max_value) + "])", ctx);
        }
        return FilterResult::success();
    }
}

