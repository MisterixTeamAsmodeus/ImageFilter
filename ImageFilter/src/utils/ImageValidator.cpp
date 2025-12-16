#include <utils/ImageValidator.h>
#include <ImageProcessor.h>
#include <utils/FilterValidator.h>
#include <utils/SafeMath.h>
#include <algorithm>

namespace ImageValidator
{
    FilterResult validateBasic(const ImageProcessor& image)
    {
        if (!image.isValid())
        {
            return FilterResult::failure(FilterError::InvalidImage, "Изображение не загружено");
        }

        const int width = image.getWidth();
        const int height = image.getHeight();
        const int channels = image.getChannels();

        return FilterValidator::validateImageSize(width, height, channels);
    }

    FilterResult validateDataIntegrity(const ImageProcessor& image, bool deep_check)
    {
        // Сначала базовая проверка
        auto basic_result = validateBasic(image);
        if (basic_result.hasError())
        {
            return basic_result;
        }

        const int width = image.getWidth();
        const int height = image.getHeight();
        const int channels = image.getChannels();
        const uint8_t* data = image.getData();

        // Проверка указателя
        auto pointer_result = validateDataPointer(data, width, height, channels);
        if (pointer_result.hasError())
        {
            return pointer_result;
        }

        // Проверка размера данных
        auto size_result = validateDataSize(image);
        if (size_result.hasError())
        {
            return size_result;
        }

        // Глубокая проверка всех пикселей (если запрошена)
        if (deep_check)
        {
            auto pixel_result = validatePixelValues(image, 1.0);
            if (pixel_result.hasError())
            {
                return pixel_result;
            }
        }

        return FilterResult::success();
    }

    FilterResult validateDataPointer(const uint8_t* data, int width, int height, int channels)
    {
        if (data == nullptr)
        {
            ErrorContext ctx = ErrorContext::withImage(width, height, channels);
            return FilterResult::failure(FilterError::InvalidImageData,
                                       "Указатель на данные изображения равен nullptr", ctx);
        }

        return FilterResult::success();
    }

    FilterResult validatePixelValues(const ImageProcessor& image, double sample_rate)
    {
        if (!image.isValid())
        {
            return FilterResult::failure(FilterError::InvalidImage, "Изображение не загружено");
        }

        const int width = image.getWidth();
        const int height = image.getHeight();
        const int channels = image.getChannels();
        const uint8_t* data = image.getData();

        if (data == nullptr)
        {
            ErrorContext ctx = ErrorContext::withImage(width, height, channels);
            return FilterResult::failure(FilterError::InvalidImageData,
                                       "Указатель на данные изображения равен nullptr", ctx);
        }

        // Вычисляем количество пикселей для проверки
        size_t total_pixels = 0;
        if (!SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), total_pixels))
        {
            ErrorContext ctx = ErrorContext::withImage(width, height, channels);
            return FilterResult::failure(FilterError::ArithmeticOverflow,
                                       "Переполнение при вычислении количества пикселей", ctx);
        }

        // Для полной проверки проверяем все пиксели
        if (sample_rate >= 1.0)
        {
            const size_t total_elements = total_pixels * static_cast<size_t>(channels);
            for (size_t i = 0; i < total_elements; ++i)
            {
                // Значения uint8_t всегда в диапазоне [0, 255], но проверяем на всякий случай
                // (на случай если данные были повреждены)
                if (data[i] > 255)
                {
                    ErrorContext ctx = ErrorContext::withImage(width, height, channels);
                    return FilterResult::failure(FilterError::InvalidImageData,
                                               "Найдено некорректное значение пикселя: " +
                                               std::to_string(static_cast<int>(data[i])) +
                                               " (ожидается диапазон [0, 255])", ctx);
                }
            }
        }
        else
        {
            // Выборочная проверка
            const size_t samples_to_check = static_cast<size_t>(static_cast<double>(total_pixels) * sample_rate);
            if (samples_to_check == 0)
            {
                // Если sample_rate слишком мал, проверяем хотя бы несколько пикселей
                const size_t min_samples = std::min(total_pixels, static_cast<size_t>(100));
                for (size_t i = 0; i < min_samples; ++i)
                {
                    const size_t pixel_index = (i * total_pixels) / min_samples;
                    for (int c = 0; c < channels; ++c)
                    {
                        const size_t index = pixel_index * static_cast<size_t>(channels) + static_cast<size_t>(c);
                        if (data[index] > 255)
                        {
                            ErrorContext ctx = ErrorContext::withImage(width, height, channels);
                            return FilterResult::failure(FilterError::InvalidImageData,
                                                       "Найдено некорректное значение пикселя: " +
                                                       std::to_string(static_cast<int>(data[index])) +
                                                       " (ожидается диапазон [0, 255])", ctx);
                        }
                    }
                }
            }
            else
            {
                // Используем равномерную выборку
                const size_t step = total_pixels / samples_to_check;
                for (size_t i = 0; i < samples_to_check; ++i)
                {
                    const size_t pixel_index = i * step;
                    for (int c = 0; c < channels; ++c)
                    {
                        const size_t index = pixel_index * static_cast<size_t>(channels) + static_cast<size_t>(c);
                        if (index < total_pixels * static_cast<size_t>(channels))
                        {
                            if (data[index] > 255)
                            {
                                ErrorContext ctx = ErrorContext::withImage(width, height, channels);
                                return FilterResult::failure(FilterError::InvalidImageData,
                                                           "Найдено некорректное значение пикселя: " +
                                                           std::to_string(static_cast<int>(data[index])) +
                                                           " (ожидается диапазон [0, 255])", ctx);
                            }
                        }
                    }
                }
            }
        }

        return FilterResult::success();
    }

    FilterResult validateDataSize(const ImageProcessor& image)
    {
        if (!image.isValid())
        {
            return FilterResult::failure(FilterError::InvalidImage, "Изображение не загружено");
        }

        const int width = image.getWidth();
        const int height = image.getHeight();
        const int channels = image.getChannels();
        const uint8_t* data = image.getData();

        if (data == nullptr)
        {
            ErrorContext ctx = ErrorContext::withImage(width, height, channels);
            return FilterResult::failure(FilterError::InvalidImageData,
                                       "Указатель на данные изображения равен nullptr", ctx);
        }

        // Вычисляем ожидаемый размер данных с защитой от переполнения
        size_t expected_size = 0;
        auto size_result = FilterValidator::validateBufferSize(width, height, channels, expected_size);
        if (size_result.hasError())
        {
            return size_result;
        }

        // Проверяем, что данные доступны (базовая проверка)
        // Примечание: мы не можем проверить реальный размер выделенной памяти,
        // так как это зависит от реализации stbi_image_free, но можем проверить,
        // что указатель не nullptr и размеры корректны

        return FilterResult::success();
    }

    FilterResult validateNotEmpty(const ImageProcessor& image)
    {
        if (!image.isValid())
        {
            return FilterResult::failure(FilterError::InvalidImage, "Изображение не загружено");
        }

        const int width = image.getWidth();
        const int height = image.getHeight();

        if (width == 0 || height == 0)
        {
            ErrorContext ctx = ErrorContext::withImage(width, height, image.getChannels());
            return FilterResult::failure(FilterError::EmptyImage,
                                       "Изображение пустое (размер: " +
                                       std::to_string(width) + "x" + std::to_string(height) + ")", ctx);
        }

        return FilterResult::success();
    }

    FilterResult validateFull(const ImageProcessor& image, bool deep_check)
    {
        // Базовая проверка
        auto basic_result = validateBasic(image);
        if (basic_result.hasError())
        {
            return basic_result;
        }

        // Проверка на пустоту
        auto not_empty_result = validateNotEmpty(image);
        if (not_empty_result.hasError())
        {
            return not_empty_result;
        }

        // Проверка целостности данных
        auto integrity_result = validateDataIntegrity(image, deep_check);
        if (integrity_result.hasError())
        {
            return integrity_result;
        }

        // Выборочная проверка значений пикселей (если не выполняется глубокая проверка)
        if (!deep_check)
        {
            auto pixel_result = validatePixelValues(image, 0.1);
            if (pixel_result.hasError())
            {
                return pixel_result;
            }
        }

        return FilterResult::success();
    }
}

