#include <utils/ImageConverter.h>
#include <utils/FilterResult.h>
#include <utils/SafeMath.h>

#include <cstdlib>
#include <cerrno>

FilterResult ImageConverter::convertRGBAToRGB(const uint8_t* rgba_data,
                                             int width,
                                             int height,
                                             uint8_t* rgb_data)
{
    if (rgba_data == nullptr || rgb_data == nullptr)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, 4);
        return FilterResult::failure(FilterError::InvalidImage, 
                                   "Данные изображения не заданы", ctx);
    }

    if (width <= 0 || height <= 0)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, 4);
        return FilterResult::failure(FilterError::InvalidSize, 
                                   "Некорректный размер изображения", ctx);
    }

    // Преобразуем RGBA в RGB
    // Используем альфа-канал для композиции (alpha blending с белым фоном)
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            size_t y_width_sum = 0;
            size_t pixel_index = 0;
            if (!SafeMath::safeMultiply(static_cast<size_t>(y), static_cast<size_t>(width), y_width_sum) ||
                !SafeMath::safeAdd(y_width_sum, static_cast<size_t>(x), pixel_index))
            {
                ErrorContext ctx = ErrorContext::withImage(width, height, 4);
                return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                           "Размер изображения слишком большой", ctx);
            }
            
            size_t rgba_offset = 0;
            size_t rgb_offset = 0;
            if (!SafeMath::safeMultiply(pixel_index, static_cast<size_t>(4), rgba_offset) ||
                !SafeMath::safeMultiply(pixel_index, static_cast<size_t>(3), rgb_offset))
            {
                ErrorContext ctx = ErrorContext::withImage(width, height, 4);
                return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                           "Размер изображения слишком большой", ctx);
            }
            
            const auto alpha = static_cast<float>(rgba_data[rgba_offset + 3]) / 255.0f;
            
            // Композиция с белым фоном: result = alpha * color + (1 - alpha) * white
            rgb_data[rgb_offset + 0] = static_cast<uint8_t>(
                alpha * static_cast<float>(rgba_data[rgba_offset + 0]) + (1.0f - alpha) * 255.0f);
            rgb_data[rgb_offset + 1] = static_cast<uint8_t>(
                alpha * static_cast<float>(rgba_data[rgba_offset + 1]) + (1.0f - alpha) * 255.0f);
            rgb_data[rgb_offset + 2] = static_cast<uint8_t>(
                alpha * static_cast<float>(rgba_data[rgba_offset + 2]) + (1.0f - alpha) * 255.0f);
        }
    }

    return FilterResult::success();
}

