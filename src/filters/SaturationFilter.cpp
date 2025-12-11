#include <filters/SaturationFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <algorithm>

FilterResult SaturationFilter::apply(ImageProcessor& image)
{
    if (!image.isValid())
    {
        return FilterResult::failure(FilterError::InvalidImage, "Изображение не загружено");
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();

    // Валидация размеров изображения
    if (width <= 0 || height <= 0)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        ctx.filter_params = "factor=" + std::to_string(factor_);
        return FilterResult::failure(FilterError::InvalidSize,
                                     "Размер изображения должен быть больше нуля", ctx);
    }

    if (channels != 3 && channels != 4)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        ctx.filter_params = "factor=" + std::to_string(factor_);
        return FilterResult::failure(FilterError::InvalidChannels, 
                                     "Ожидается 3 канала (RGB) или 4 канала (RGBA), получено: " + std::to_string(channels),
                                     ctx);
    }

    // Валидация параметра фильтра
    if (factor_ < 0.0)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        ctx.filter_params = "factor=" + std::to_string(factor_);
        return FilterResult::failure(FilterError::InvalidFactor,
                                     "Коэффициент насыщенности должен быть >= 0, получено: " + std::to_string(factor_),
                                     ctx);
    }

    auto* data = image.getData();
    const auto factor = static_cast<int>(factor_ * 65536); // Масштабируем для целочисленной арифметики

    ParallelImageProcessor::processRowsParallel(
        height,
        [width, channels, data, factor](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                const auto row_offset = static_cast<size_t>(y) * static_cast<size_t>(width) * static_cast<size_t>(channels);

                for (int x = 0; x < width; ++x)
                {
                    // Коэффициенты для преобразования RGB в градации серого
                    constexpr int R_COEFF = 19595; // 0.299 * 65536
                    constexpr int G_COEFF = 38470; // 0.587 * 65536
                    constexpr int B_COEFF = 7471; // 0.114 * 65536
                    const auto pixel_offset = row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels);

                    const auto r = static_cast<int>(data[pixel_offset + 0]);
                    const auto g = static_cast<int>(data[pixel_offset + 1]);
                    const auto b = static_cast<int>(data[pixel_offset + 2]);

                    // Вычисляем яркость (градации серого)
                    const auto gray = (R_COEFF * r + G_COEFF * g + B_COEFF * b) >> 16;

                    // Интерполируем между серым и оригинальным цветом
                    const auto new_r = gray + (((r - gray) * factor) >> 16);
                    const auto new_g = gray + (((g - gray) * factor) >> 16);
                    const auto new_b = gray + (((b - gray) * factor) >> 16);

                    data[pixel_offset + 0] = static_cast<uint8_t>(std::max(0, std::min(255, new_r)));
                    data[pixel_offset + 1] = static_cast<uint8_t>(std::max(0, std::min(255, new_g)));
                    data[pixel_offset + 2] = static_cast<uint8_t>(std::max(0, std::min(255, new_b)));
                }
            }
        }
    );

    return FilterResult::success();
}

std::string SaturationFilter::getName() const
{
    return "saturation";
}

std::string SaturationFilter::getDescription() const
{
    return "Изменение насыщенности";
}

std::string SaturationFilter::getCategory() const
{
    return "Цветовой";
}



