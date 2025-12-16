#include <filters/SepiaFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/FilterValidationHelper.h>
#include <algorithm>

FilterResult SepiaFilter::apply(ImageProcessor& image)
{
    // Базовая валидация изображения
    auto validation_result = FilterValidationHelper::validateImageOnly(image);
    if (validation_result.hasError())
    {
        return validation_result;
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();
    auto* data = image.getData();

    ParallelImageProcessor::processRowsParallel(
        height,
        [width, channels, data](
        int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                const auto row_offset = static_cast<size_t>(y) * static_cast<size_t>(width) * static_cast<size_t>(channels);

                for (int x = 0; x < width; ++x)
                {
                    // Коэффициенты для эффекта сепии (масштабированы на 65536 для целочисленной арифметики)
                    constexpr int R_TO_R = 25772; // 0.393 * 65536
                    constexpr int G_TO_R = 50400; // 0.769 * 65536
                    constexpr int B_TO_R = 12390; // 0.189 * 65536

                    constexpr int R_TO_G = 22878; // 0.349 * 65536
                    constexpr int G_TO_G = 44958; // 0.686 * 65536
                    constexpr int B_TO_G = 11010; // 0.168 * 65536

                    constexpr int R_TO_B = 17826; // 0.272 * 65536
                    constexpr int G_TO_B = 35000; // 0.534 * 65536
                    constexpr int B_TO_B = 8584; // 0.131 * 65536

                    const auto pixel_offset = row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels);

                    const auto r = static_cast<int>(data[pixel_offset + 0]);
                    const auto g = static_cast<int>(data[pixel_offset + 1]);
                    const auto b = static_cast<int>(data[pixel_offset + 2]);

                    // Применяем матрицу преобразования сепии
                    const auto new_r = (R_TO_R * r + G_TO_R * g + B_TO_R * b) >> 16;
                    const auto new_g = (R_TO_G * r + G_TO_G * g + B_TO_G * b) >> 16;
                    const auto new_b = (R_TO_B * r + G_TO_B * g + B_TO_B * b) >> 16;

                    // Ограничиваем значения диапазоном [0, 255]
                    data[pixel_offset + 0] = static_cast<uint8_t>(std::max(0, std::min(255, new_r)));
                    data[pixel_offset + 1] = static_cast<uint8_t>(std::max(0, std::min(255, new_g)));
                    data[pixel_offset + 2] = static_cast<uint8_t>(std::max(0, std::min(255, new_b)));
                }
            }
        }
    );

    return FilterResult::success();
}

std::string SepiaFilter::getName() const
{
    return "sepia";
}

std::string SepiaFilter::getDescription() const
{
    return "Эффект сепии (винтажный)";
}

std::string SepiaFilter::getCategory() const
{
    return "Цветовой";
}

bool SepiaFilter::supportsInPlace() const noexcept
{
    return true;
}



