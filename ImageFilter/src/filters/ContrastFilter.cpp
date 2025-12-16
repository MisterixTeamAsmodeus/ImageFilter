#include <filters/ContrastFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/FilterValidator.h>
#include <utils/FilterValidationHelper.h>
#include <utils/PixelOffsetUtils.h>
#include <utils/SafeMath.h>
#include <algorithm>

FilterResult ContrastFilter::apply(ImageProcessor& image) {
    // Валидация параметра фильтра
    auto factor_result = FilterValidator::validateFactor(factor_);
    
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
    const auto factor = static_cast<int>(factor_ * 65536);  // Масштабируем для целочисленной арифметики
    constexpr int MIDDLE = 128;
    constexpr int color_channels = 3; // Обрабатываем только RGB каналы

    ParallelImageProcessor::processRowsParallel(
        height,
        [width, channels, data, factor](int start_row, int end_row) {
            for (int y = start_row; y < end_row; ++y) {
                // Вычисляем смещение строки с защитой от переполнения
                size_t row_offset = 0;
                if (!PixelOffsetUtils::computeRowOffset(y, width, channels, row_offset))
                {
                    // Пропускаем строку при переполнении
                    continue;
                }
                
                for (int x = 0; x < width; ++x) {
                    // Вычисляем смещение пикселя с защитой от переполнения
                    size_t pixel_offset = 0;
                    if (!PixelOffsetUtils::computePixelOffset(row_offset, x, channels, pixel_offset))
                    {
                        // Пропускаем пиксель при переполнении
                        continue;
                    }
                    
                    // Применяем контраст только к цветовым каналам (RGB)
                    // Альфа-канал сохраняется без изменений
                    for (int c = 0; c < color_channels; ++c) {
                        size_t channel_offset = 0;
                        if (!PixelOffsetUtils::computeChannelOffset(pixel_offset, c, channel_offset))
                        {
                            continue; // Пропускаем при переполнении
                        }
                        
                        // Проверка границ
                        const size_t row_end = row_offset + static_cast<size_t>(width) * static_cast<size_t>(channels);
                        if (channel_offset >= row_end)
                        {
                            continue; // Защита от выхода за границы
                        }
                        
                        const auto old_value = static_cast<int>(data[channel_offset]);
                        const auto diff = old_value - MIDDLE;
                        // Вычисляем новое значение с проверкой на переполнение
                        // factor уже масштабирован на 65536, поэтому используем int64_t для промежуточных вычислений
                        const int64_t diff_64 = static_cast<int64_t>(diff);
                        const int64_t factor_64 = static_cast<int64_t>(factor);
                        int64_t diff_factor = diff_64 * factor_64;
                        // Проверка на переполнение (приблизительная)
                        constexpr int64_t max_safe = (static_cast<int64_t>(INT_MAX) << 16);
                        constexpr int64_t min_safe = (static_cast<int64_t>(INT_MIN) << 16);
                        if (diff_factor > max_safe)
                        {
                            diff_factor = max_safe;
                        }
                        else if (diff_factor < min_safe)
                        {
                            diff_factor = min_safe;
                        }
                        const auto new_value = static_cast<int>((diff_factor >> 16) + MIDDLE);
                        data[channel_offset] = static_cast<uint8_t>(std::max(0, std::min(255, new_value)));
                    }
                    // Альфа-канал (pixel_offset + 3) не изменяется, если channels == 4
                }
            }
        }
    );

    return FilterResult::success();
}

std::string ContrastFilter::getName() const
{
    return "contrast";
}

std::string ContrastFilter::getDescription() const
{
    return "Изменение контрастности";
}

std::string ContrastFilter::getCategory() const
{
    return "Цветовой";
}

bool ContrastFilter::supportsInPlace() const noexcept
{
    return true;
}




