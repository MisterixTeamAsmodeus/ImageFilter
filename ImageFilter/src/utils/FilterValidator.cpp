#include <utils/FilterValidator.h>
#include <utils/SafeMath.h>
#include <algorithm>

namespace FilterValidator
{
    FilterResult validateRadius(int radius, int min_radius, int max_radius, int width, int height)
    {
        if (radius < min_radius)
        {
            ErrorContext ctx;
            ctx.filter_params = "radius=" + std::to_string(radius);
            return FilterResult::failure(FilterError::InvalidRadius,
                                       "Радиус должен быть >= " + std::to_string(min_radius) +
                                       ", получено: " + std::to_string(radius), ctx);
        }

        if (radius > max_radius)
        {
            ErrorContext ctx;
            ctx.filter_params = "radius=" + std::to_string(radius);
            return FilterResult::failure(FilterError::InvalidRadius,
                                       "Радиус должен быть <= " + std::to_string(max_radius) +
                                       ", получено: " + std::to_string(radius), ctx);
        }

        // Проверка, что радиус не превышает размер изображения
        if (width > 0 && height > 0)
        {
            const int max_dimension = std::max(width, height);
            if (radius > max_dimension / 2)
            {
                ErrorContext ctx = ErrorContext::withImage(width, height, 0);
                ctx.filter_params = "radius=" + std::to_string(radius);
                return FilterResult::failure(FilterError::InvalidRadius,
                                           "Радиус (" + std::to_string(radius) + 
                                           ") слишком большой для изображения размером " +
                                           std::to_string(width) + "x" + std::to_string(height) +
                                           " (максимум: " + std::to_string(max_dimension / 2) + ")", ctx);
            }
        }

        return FilterResult::success();
    }

    FilterResult validateRadius(double radius, double min_radius, double max_radius, int width, int height)
    {
        if (radius < min_radius)
        {
            ErrorContext ctx;
            ctx.filter_params = "radius=" + std::to_string(radius);
            return FilterResult::failure(FilterError::InvalidRadius,
                                       "Радиус должен быть >= " + std::to_string(min_radius) +
                                       ", получено: " + std::to_string(radius), ctx);
        }

        if (radius > max_radius)
        {
            ErrorContext ctx;
            ctx.filter_params = "radius=" + std::to_string(radius);
            return FilterResult::failure(FilterError::InvalidRadius,
                                       "Радиус должен быть <= " + std::to_string(max_radius) +
                                       ", получено: " + std::to_string(radius), ctx);
        }

        // Проверка, что радиус не превышает размер изображения
        if (width > 0 && height > 0)
        {
            const int max_dimension = std::max(width, height);
            const double max_radius_for_image = static_cast<double>(max_dimension) / 2.0;
            if (radius > max_radius_for_image)
            {
                ErrorContext ctx = ErrorContext::withImage(width, height, 0);
                ctx.filter_params = "radius=" + std::to_string(radius);
                return FilterResult::failure(FilterError::InvalidRadius,
                                           "Радиус (" + std::to_string(radius) + 
                                           ") слишком большой для изображения размером " +
                                           std::to_string(width) + "x" + std::to_string(height) +
                                           " (максимум: " + std::to_string(max_radius_for_image) + ")", ctx);
            }
        }

        return FilterResult::success();
    }

    FilterResult validateFactor(double factor, double min_factor, double max_factor)
    {
        if (factor < min_factor || factor > max_factor)
        {
            ErrorContext ctx;
            ctx.filter_params = "factor=" + std::to_string(factor);
            return FilterResult::failure(FilterError::InvalidFactor,
                                       "Коэффициент должен быть в диапазоне [" +
                                       std::to_string(min_factor) + ", " + std::to_string(max_factor) +
                                       "], получено: " + std::to_string(factor), ctx);
        }

        // Проверка на переполнение при умножении
        if (factor > 0.0 && factor < std::numeric_limits<double>::min())
        {
            ErrorContext ctx;
            ctx.filter_params = "factor=" + std::to_string(factor);
            return FilterResult::failure(FilterError::ArithmeticOverflow,
                                       "Коэффициент слишком мал и может привести к переполнению", ctx);
        }

        if (factor > std::numeric_limits<double>::max() / 255.0)
        {
            ErrorContext ctx;
            ctx.filter_params = "factor=" + std::to_string(factor);
            return FilterResult::failure(FilterError::ArithmeticOverflow,
                                       "Коэффициент слишком велик и может привести к переполнению", ctx);
        }

        return FilterResult::success();
    }

    FilterResult validateThreshold(int threshold, int min_threshold, int max_threshold)
    {
        if (threshold < min_threshold || threshold > max_threshold)
        {
            ErrorContext ctx;
            ctx.filter_params = "threshold=" + std::to_string(threshold);
            return FilterResult::failure(FilterError::InvalidThreshold,
                                       "Порог должен быть в диапазоне [" +
                                       std::to_string(min_threshold) + ", " + std::to_string(max_threshold) +
                                       "], получено: " + std::to_string(threshold), ctx);
        }

        return FilterResult::success();
    }

    FilterResult validateIntensity(double intensity, double min_intensity, double max_intensity)
    {
        if (intensity < min_intensity || intensity > max_intensity)
        {
            ErrorContext ctx;
            ctx.filter_params = "intensity=" + std::to_string(intensity);
            return FilterResult::failure(FilterError::ParameterOutOfRange,
                                       "Интенсивность должна быть в диапазоне [" +
                                       std::to_string(min_intensity) + ", " + std::to_string(max_intensity) +
                                       "], получено: " + std::to_string(intensity), ctx);
        }

        return FilterResult::success();
    }

    FilterResult validateQuality(int quality, int min_quality, int max_quality)
    {
        if (quality < min_quality || quality > max_quality)
        {
            ErrorContext ctx;
            ctx.filter_params = "quality=" + std::to_string(quality);
            return FilterResult::failure(FilterError::InvalidQuality,
                                       "Качество должно быть в диапазоне [" +
                                       std::to_string(min_quality) + ", " + std::to_string(max_quality) +
                                       "], получено: " + std::to_string(quality), ctx);
        }

        return FilterResult::success();
    }

    FilterResult validateAngle(double angle, double min_angle, double max_angle)
    {
        if (angle < min_angle || angle > max_angle)
        {
            ErrorContext ctx;
            ctx.filter_params = "angle=" + std::to_string(angle);
            return FilterResult::failure(FilterError::InvalidAngle,
                                       "Угол должен быть в диапазоне [" +
                                       std::to_string(min_angle) + ", " + std::to_string(max_angle) +
                                       "], получено: " + std::to_string(angle), ctx);
        }

        return FilterResult::success();
    }

    FilterResult validateKernelSize(int kernel_size, int min_size, int max_size)
    {
        if (kernel_size < min_size || kernel_size > max_size)
        {
            ErrorContext ctx;
            ctx.filter_params = "kernel_size=" + std::to_string(kernel_size);
            return FilterResult::failure(FilterError::InvalidKernelSize,
                                       "Размер ядра должен быть в диапазоне [" +
                                       std::to_string(min_size) + ", " + std::to_string(max_size) +
                                       "], получено: " + std::to_string(kernel_size), ctx);
        }

        // Проверка, что размер ядра нечетный (для большинства фильтров)
        if (kernel_size % 2 == 0)
        {
            ErrorContext ctx;
            ctx.filter_params = "kernel_size=" + std::to_string(kernel_size);
            return FilterResult::failure(FilterError::InvalidKernelSize,
                                       "Размер ядра должен быть нечетным, получено: " +
                                       std::to_string(kernel_size), ctx);
        }

        return FilterResult::success();
    }

    FilterResult validateImageSize(int width, int height, int channels)
    {
        if (width <= 0)
        {
            ErrorContext ctx = ErrorContext::withImage(width, height, channels);
            return FilterResult::failure(FilterError::InvalidWidth,
                                       "Ширина изображения должна быть больше нуля, получено: " +
                                       std::to_string(width), ctx);
        }

        if (height <= 0)
        {
            ErrorContext ctx = ErrorContext::withImage(width, height, channels);
            return FilterResult::failure(FilterError::InvalidHeight,
                                       "Высота изображения должна быть больше нуля, получено: " +
                                       std::to_string(height), ctx);
        }

        if (channels != 3 && channels != 4)
        {
            ErrorContext ctx = ErrorContext::withImage(width, height, channels);
            return FilterResult::failure(FilterError::InvalidChannels,
                                       "Количество каналов должно быть 3 (RGB) или 4 (RGBA), получено: " +
                                       std::to_string(channels), ctx);
        }

        // Проверка на переполнение при вычислении размера изображения
        size_t width_height_product = 0;
        if (!SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), width_height_product))
        {
            ErrorContext ctx = ErrorContext::withImage(width, height, channels);
            return FilterResult::failure(FilterError::ArithmeticOverflow,
                                       "Размер изображения слишком большой (переполнение при вычислении width * height)", ctx);
        }

        size_t image_size = 0;
        if (!SafeMath::safeMultiply(width_height_product, static_cast<size_t>(channels), image_size))
        {
            ErrorContext ctx = ErrorContext::withImage(width, height, channels);
            return FilterResult::failure(FilterError::ArithmeticOverflow,
                                       "Размер изображения слишком большой (переполнение при вычислении размера буфера)", ctx);
        }

        return FilterResult::success();
    }

    FilterResult validateBufferSize(int width, int height, int channels, size_t& buffer_size)
    {
        // Сначала валидируем размеры изображения
        auto size_result = validateImageSize(width, height, channels);
        if (size_result.hasError())
        {
            return size_result;
        }

        // Вычисляем размер буфера с защитой от переполнения
        size_t width_height_product = 0;
        if (!SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), width_height_product))
        {
            ErrorContext ctx = ErrorContext::withImage(width, height, channels);
            return FilterResult::failure(FilterError::ArithmeticOverflow,
                                       "Переполнение при вычислении width * height", ctx);
        }

        if (!SafeMath::safeMultiply(width_height_product, static_cast<size_t>(channels), buffer_size))
        {
            ErrorContext ctx = ErrorContext::withImage(width, height, channels);
            return FilterResult::failure(FilterError::ArithmeticOverflow,
                                       "Переполнение при вычислении размера буфера", ctx);
        }

        return FilterResult::success();
    }
}

