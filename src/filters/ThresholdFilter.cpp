#include <filters/ThresholdFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>

FilterResult ThresholdFilter::apply(ImageProcessor& image)
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
        ctx.filter_params = "threshold=" + std::to_string(threshold_);
        return FilterResult::failure(FilterError::InvalidSize,
                                     "Размер изображения должен быть больше нуля", ctx);
    }

    if (channels != 3 && channels != 4)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        ctx.filter_params = "threshold=" + std::to_string(threshold_);
        return FilterResult::failure(FilterError::InvalidChannels, 
                                     "Ожидается 3 канала (RGB) или 4 канала (RGBA), получено: " + std::to_string(channels),
                                     ctx);
    }
    
    // Валидация параметра фильтра
    if (threshold_ < 0 || threshold_ > 255)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        ctx.filter_params = "threshold=" + std::to_string(threshold_);
        return FilterResult::failure(FilterError::InvalidThreshold, 
                                     "Порог должен быть в диапазоне [0, 255], получено: " + std::to_string(threshold_),
                                     ctx);
    }

    auto* data = image.getData();

    ParallelImageProcessor::processRowsParallel(
        height,
        [width, channels, data, this](int start_row, int end_row)
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

                    // Вычисляем яркость
                    const auto gray = (R_COEFF * r + G_COEFF * g + B_COEFF * b) >> 16;

                    // Применяем порог
                    const auto value = (gray >= threshold_) ? 255 : 0;

                    // Применяем порог только к цветовым каналам
                    // Альфа-канал сохраняется без изменений
                    data[pixel_offset + 0] = static_cast<uint8_t>(value);
                    data[pixel_offset + 1] = static_cast<uint8_t>(value);
                    data[pixel_offset + 2] = static_cast<uint8_t>(value);
                    // Альфа-канал (pixel_offset + 3) не изменяется, если channels == 4
                }
            }
        }
    );

    return FilterResult::success();
}

std::string ThresholdFilter::getName() const
{
    return "threshold";
}

std::string ThresholdFilter::getDescription() const
{
    return "Пороговая бинаризация";
}

std::string ThresholdFilter::getCategory() const
{
    return "Стилистический";
}



