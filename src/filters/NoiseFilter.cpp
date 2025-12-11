#include <filters/NoiseFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <algorithm>
#include <random>

FilterResult NoiseFilter::apply(ImageProcessor& image)
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
        ctx.filter_params = "intensity=" + std::to_string(intensity_);
        return FilterResult::failure(FilterError::InvalidSize,
                                     "Размер изображения должен быть больше нуля", ctx);
    }

    if (channels != 3 && channels != 4)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        ctx.filter_params = "intensity=" + std::to_string(intensity_);
        return FilterResult::failure(FilterError::InvalidChannels, 
                                     "Ожидается 3 канала (RGB) или 4 канала (RGBA), получено: " + std::to_string(channels),
                                     ctx);
    }
    
    // Валидация параметра фильтра
    if (intensity_ < 0.0 || intensity_ > 1.0)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        ctx.filter_params = "intensity=" + std::to_string(intensity_);
        return FilterResult::failure(FilterError::ParameterOutOfRange, 
                                     "Интенсивность должна быть в диапазоне [0.0, 1.0], получено: " + std::to_string(intensity_),
                                     ctx);
    }

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
                const auto row_offset = static_cast<size_t>(y) * static_cast<size_t>(width) * static_cast<size_t>(channels);

                for (int x = 0; x < width; ++x)
                {
                    const auto pixel_offset = row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels);

                    for (int c = 0; c < channels; ++c)
                    {
                        const auto old_value = static_cast<int>(data[pixel_offset + static_cast<size_t>(c)]);
                        const auto noise = local_dist(local_gen);
                        const auto new_value = old_value + noise;
                        data[pixel_offset + static_cast<size_t>(c)] = static_cast<uint8_t>(std::max(0, std::min(255, new_value)));
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

