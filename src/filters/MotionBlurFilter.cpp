#include <filters/MotionBlurFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/BorderHandler.h>
#include <utils/LookupTables.h>
#include <algorithm>
#include <vector>
#include <numbers>

FilterResult MotionBlurFilter::apply(ImageProcessor& image)
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
        ctx.filter_params = "length=" + std::to_string(length_) + ", angle=" + std::to_string(angle_);
        return FilterResult::failure(FilterError::InvalidSize,
                                     "Размер изображения должен быть больше нуля", ctx);
    }

    if (channels != 3 && channels != 4)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        ctx.filter_params = "length=" + std::to_string(length_) + ", angle=" + std::to_string(angle_);
        return FilterResult::failure(FilterError::InvalidChannels, 
                                     "Ожидается 3 канала (RGB) или 4 канала (RGBA), получено: " + std::to_string(channels),
                                     ctx);
    }
    
    // Валидация параметров фильтра
    if (length_ <= 0)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        ctx.filter_params = "length=" + std::to_string(length_) + ", angle=" + std::to_string(angle_);
        return FilterResult::failure(FilterError::InvalidParameter, 
                                     "Длина размытия должна быть > 0, получено: " + std::to_string(length_),
                                     ctx);
    }

    const auto* input_data = image.getData();
    const auto buffer_size = static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(channels);
    
    // Создаем буфер для результата
    std::vector<uint8_t> result(buffer_size);

    // Инициализируем lookup tables
    LookupTables::initialize();

    // Используем lookup table для sin/cos вместо вычислений в runtime
    const auto angle_degrees = static_cast<int>(angle_);
    const auto dx = LookupTables::cos(angle_degrees);
    const auto dy = LookupTables::sin(angle_degrees);

    ParallelImageProcessor::processRowsParallel(
        height,
        [width, height, channels, input_data, &result, this, dx, dy](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    for (int c = 0; c < channels; ++c)
                    {
                        int64_t sum = 0;
                        int count = 0;

                        // Собираем пиксели вдоль линии движения
                        for (int i = -length_ / 2; i <= length_ / 2; ++i)
                        {
                            const auto sample_x = static_cast<int>(x + i * dx);
                            const auto sample_y = static_cast<int>(y + i * dy);

                            // Обработка границ с использованием BorderHandler
                            const auto clamped_x = border_handler_.getX(sample_x, width);
                            const auto clamped_y = border_handler_.getY(sample_y, height);

                            const auto pixel_offset = (static_cast<size_t>(clamped_y) * static_cast<size_t>(width) + static_cast<size_t>(clamped_x)) * static_cast<size_t>(channels) +
                                static_cast<size_t>(c);
                            sum += static_cast<int>(input_data[pixel_offset]);
                            ++count;
                        }

                        // Защита от деления на ноль
                        if (count == 0)
                        {
                            result[(static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * static_cast<size_t>(channels) + static_cast<size_t>(c)] = input_data[(static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * static_cast<size_t>(channels) + static_cast<size_t>(c)];
                            continue;
                        }
                        
                        // Защита от переполнения при делении
                        const auto avg = (count > 0) ? static_cast<int>(sum / static_cast<int64_t>(count)) : 0;
                        const auto pixel_offset = (static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * static_cast<size_t>(channels) + static_cast<size_t>(c);
                        result[pixel_offset] = static_cast<uint8_t>(std::max(0, std::min(255, avg)));
                    }
                }
            }
        }
    );

    // Копируем результат обратно
    auto* data = image.getData();
    std::ranges::copy(result, data);

    return FilterResult::success();
}

std::string MotionBlurFilter::getName() const
{
    return "motion_blur";
}

std::string MotionBlurFilter::getDescription() const
{
    return "Размытие движения";
}

std::string MotionBlurFilter::getCategory() const
{
    return "Размытие и шум";
}



