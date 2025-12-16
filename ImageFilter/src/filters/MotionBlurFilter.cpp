#include <filters/MotionBlurFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/FilterValidator.h>
#include <utils/FilterValidationHelper.h>
#include <utils/BorderHandler.h>
#include <utils/LookupTables.h>
#include <utils/IBufferPool.h>
#include <utils/SafeMath.h>
#include <algorithm>
#include <vector>
#include <numbers>

FilterResult MotionBlurFilter::apply(ImageProcessor& image)
{
    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();

    // Валидация параметров фильтра
    FilterResult length_result = FilterResult::success();
    if (length_ <= 0)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        ctx.withFilterParam("length", length_).withFilterParam("angle", angle_);
        length_result = FilterResult::failure(FilterError::InvalidParameter, 
                                     "Длина размытия должна быть > 0, получено: " + std::to_string(length_),
                                     ctx);
    }
    
    // Валидация изображения и параметра с автоматическим добавлением контекста
    auto validation_result = FilterValidationHelper::validateImageAndParam(
        image, length_result, "length", length_);
    if (validation_result.hasError())
    {
        return validation_result;
    }

    const auto* input_data = image.getData();
    size_t width_height_product = 0;
    size_t buffer_size = 0;
    if (!SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), width_height_product) ||
        !SafeMath::safeMultiply(width_height_product, static_cast<size_t>(channels), buffer_size))
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        ctx.withFilterParam("length", length_).withFilterParam("angle", angle_);
        return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                   "Размер изображения слишком большой", ctx);
    }
    
    // Получаем буфер из пула или создаем новый
    std::vector<uint8_t> result;
    if (buffer_pool_ != nullptr)
    {
        result = buffer_pool_->acquire(buffer_size);
    }
    else
    {
        result.resize(buffer_size);
    }

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

    // Возвращаем буфер в пул для переиспользования
    if (buffer_pool_ != nullptr)
    {
        buffer_pool_->release(std::move(result));
    }

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



