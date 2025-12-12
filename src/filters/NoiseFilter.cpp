#include <filters/NoiseFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/FilterValidator.h>
#include <utils/FilterValidationHelper.h>
#include <utils/PixelOffsetUtils.h>
#include <utils/SafeMath.h>
#include <algorithm>
#include <random>

FilterResult NoiseFilter::apply(ImageProcessor& image)
{
    // Валидация параметра фильтра
    auto intensity_result = FilterValidator::validateIntensity(intensity_);
    
    // Валидация изображения и параметра с автоматическим добавлением контекста
    auto validation_result = FilterValidationHelper::validateImageAndParam(
        image, intensity_result, "intensity", intensity_);
    if (validation_result.hasError())
    {
        return validation_result;
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();
    auto* data = image.getData();
    const auto max_noise = static_cast<int>(intensity_ * 255);

    ParallelImageProcessor::processRowsParallel(
        height,
        [width, channels, data, max_noise](int start_row, int end_row)
        {
            // Генератор случайных чисел для каждого потока
            thread_local std::mt19937 local_gen(std::random_device{}());
            std::uniform_int_distribution<int> local_dist(-max_noise, max_noise);

            for (int y = start_row; y < end_row; ++y)
            {
                // Вычисляем смещение строки с защитой от переполнения
                size_t row_offset = 0;
                if (!PixelOffsetUtils::computeRowOffset(y, width, channels, row_offset))
                {
                    // Пропускаем строку при переполнении
                    continue;
                }

                for (int x = 0; x < width; ++x)
                {
                    // Вычисляем смещение пикселя с защитой от переполнения
                    size_t pixel_offset = 0;
                    if (!PixelOffsetUtils::computePixelOffset(row_offset, x, channels, pixel_offset))
                    {
                        // Пропускаем пиксель при переполнении
                        continue;
                    }

                    for (int c = 0; c < channels; ++c)
                    {
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
                        const auto noise = local_dist(local_gen);
                        // Проверка на переполнение при сложении
                        int new_value = 0;
                        if (!SafeMath::safeAdd(old_value, noise, new_value))
                        {
                            // При переполнении ограничиваем значение
                            new_value = (noise > 0) ? 255 : 0;
                        }
                        data[channel_offset] = static_cast<uint8_t>(std::max(0, std::min(255, new_value)));
                    }
                }
            }
        }
    );

    return FilterResult::success();
}

std::string NoiseFilter::getName() const
{
    return "noise";
}

std::string NoiseFilter::getDescription() const
{
    return "Добавление шума";
}

std::string NoiseFilter::getCategory() const
{
    return "Размытие и шум";
}

