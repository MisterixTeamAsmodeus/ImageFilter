#include <filters/BoxBlurFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/BorderHandler.h>
#include <algorithm>
#include <vector>

FilterResult BoxBlurFilter::apply(ImageProcessor& image)
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
        ctx.filter_params = "radius=" + std::to_string(radius_);
        return FilterResult::failure(FilterError::InvalidSize,
                                     "Размер изображения должен быть больше нуля", ctx);
    }

    if (channels != 3 && channels != 4)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        ctx.filter_params = "radius=" + std::to_string(radius_);
        return FilterResult::failure(FilterError::InvalidChannels, 
                                     "Ожидается 3 канала (RGB) или 4 канала (RGBA), получено: " + std::to_string(channels),
                                     ctx);
    }
    
    // Валидация параметра фильтра
    if (radius_ < 0)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        ctx.filter_params = "radius=" + std::to_string(radius_);
        return FilterResult::failure(FilterError::InvalidRadius, 
                                     "Радиус должен быть >= 0, получено: " + std::to_string(radius_),
                                     ctx);
    }

    const auto* input_data = image.getData();
    const auto kernel_size = 2 * radius_ + 1;
    const auto kernel_weight = 65536 / kernel_size; // Масштабированный вес для целочисленной арифметики

    const auto buffer_size = static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(channels);
    
    // Применяем separable kernel: сначала по горизонтали
    std::vector<uint8_t> horizontal_result(buffer_size);

    ParallelImageProcessor::processRowsParallel(
        height,
        width,
        [width, channels, input_data, &horizontal_result, this, kernel_weight](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                const auto row_offset = static_cast<size_t>(y) * static_cast<size_t>(width) * static_cast<size_t>(channels);

                for (int x = 0; x < width; ++x)
                {
                    for (int c = 0; c < channels; ++c)
                    {
                        int64_t sum = 0;

                        // Применяем ядро по горизонтали
                        for (int kx = -radius_; kx <= radius_; ++kx)
                        {
                            const auto sample_x = x + kx;

                            // Обработка границ с использованием BorderHandler
                            const auto clamped_x = border_handler_.getX(sample_x, width);

                            const auto pixel_index = row_offset + static_cast<size_t>(clamped_x) * static_cast<size_t>(channels) + static_cast<size_t>(c);
                            sum += static_cast<int>(input_data[pixel_index]);
                        }

                        const auto result_value = static_cast<int>((sum * kernel_weight) >> 16);
                        const auto result_index = row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels) + static_cast<size_t>(c);
                        horizontal_result[result_index] = static_cast<uint8_t>(
                            std::max(0, std::min(255, result_value)));
                    }
                }
            }
        }
    );

    // Применяем ядро по вертикали
    std::vector<uint8_t> final_result(buffer_size);

    ParallelImageProcessor::processRowsParallel(
        height,
        width,
        [width, height, channels, &horizontal_result, &final_result, this, kernel_weight](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                const auto row_offset = static_cast<size_t>(y) * static_cast<size_t>(width) * static_cast<size_t>(channels);

                for (int x = 0; x < width; ++x)
                {
                    for (int c = 0; c < channels; ++c)
                    {
                        int64_t sum = 0;

                        // Применяем ядро по вертикали
                        for (int ky = -radius_; ky <= radius_; ++ky)
                        {
                            const auto sample_y = y + ky;

                            // Обработка границ с использованием BorderHandler
                            const auto clamped_y = border_handler_.getY(sample_y, height);

                            const auto pixel_index = (static_cast<size_t>(clamped_y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * static_cast<size_t>(channels) + static_cast<size_t>(c);
                            sum += static_cast<int>(horizontal_result[pixel_index]);
                        }

                        const auto result_value = static_cast<int>((sum * kernel_weight) >> 16);
                        const auto result_index = row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels) + static_cast<size_t>(c);
                        final_result[result_index] = static_cast<uint8_t>(std::max(0, std::min(255, result_value)));
                    }
                }
            }
        }
    );

    // Копируем результат обратно
    auto* data = image.getData();
    std::ranges::copy(final_result, data);

    return FilterResult::success();
}

std::string BoxBlurFilter::getName() const
{
    return "boxblur";
}

std::string BoxBlurFilter::getDescription() const
{
    return "Размытие по прямоугольнику (Box Blur)";
}

std::string BoxBlurFilter::getCategory() const
{
    return "Размытие и шум";
}

