#include <filters/SaturationFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/FilterValidator.h>
#include <utils/FilterValidationHelper.h>
#include <utils/ColorConversionUtils.h>
#include <algorithm>

FilterResult SaturationFilter::apply(ImageProcessor& image)
{
    // Валидация параметра фильтра
    auto factor_result = FilterValidator::validateFactor(factor_, 0.0);
    
    // Валидация изображения и параметра с автоматическим добавлением контекста
    auto validation_result = FilterValidationHelper::validateImageAndParam(
        image, factor_result, "factor", factor_);
    if (validation_result.hasError())
    {
        return validation_result;
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();
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
                    const auto pixel_offset = row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels);

                    const auto r = static_cast<int>(data[pixel_offset + 0]);
                    const auto g = static_cast<int>(data[pixel_offset + 1]);
                    const auto b = static_cast<int>(data[pixel_offset + 2]);

                    // Вычисляем яркость (градации серого) используя общую утилиту
                    const auto gray = ColorConversionUtils::rgbToGrayscaleInt(r, g, b);

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



