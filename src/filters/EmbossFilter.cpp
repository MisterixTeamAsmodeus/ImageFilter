#include <filters/EmbossFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/FilterValidator.h>
#include <utils/FilterValidationHelper.h>
#include <utils/BorderHandler.h>
#include <utils/SafeMath.h>
#include <algorithm>
#include <vector>

FilterResult EmbossFilter::apply(ImageProcessor& image)
{
    // Валидация параметра фильтра
    auto strength_result = FilterValidator::validateFactor(strength_, 0.0);
    
    // Валидация изображения и параметра с автоматическим добавлением контекста
    auto validation_result = FilterValidationHelper::validateImageAndParam(
        image, strength_result, "strength", strength_);
    if (validation_result.hasError())
    {
        return validation_result;
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();
    const auto* input_data = image.getData();
    size_t width_height_product = 0;
    size_t buffer_size = 0;
    if (!SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), width_height_product) ||
        !SafeMath::safeMultiply(width_height_product, static_cast<size_t>(channels), buffer_size))
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        ctx.withFilterParam("strength", strength_);
        return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                   "Размер изображения слишком большой", ctx);
    }
    
    // Создаем буфер для результата
    std::vector<uint8_t> result(buffer_size);

    // Базовое ядро рельефа при strength = 1.0:
    //  -2  -1   0
    //  -1   1   1
    //   0   1   2
    //
    // При strength != 1.0 ядро масштабируется для плавной регулировки эффекта
    constexpr int BASE_KERNEL[3][3] = {
        {-2, -1, 0},
        {-1, 1, 1},
        {0, 1, 2}
    };

    // Сохраняем strength в локальную переменную для захвата в лямбде
    const double strength = strength_;

    ParallelImageProcessor::processRowsParallel(
        height,
        width,
        [width, height, channels, input_data, &result, strength, this](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    for (int c = 0; c < channels; ++c)
                    {
                        double sum = 0.0;

                        // Применяем ядро рельефа с учетом силы эффекта
                        for (int ky = -1; ky <= 1; ++ky)
                        {
                            for (int kx = -1; kx <= 1; ++kx)
                            {
                                const auto px = x + kx;
                                const auto py = y + ky;

                                // Обработка границ с использованием BorderHandler
                                const auto clamped_x = border_handler_.getX(px, width);
                                const auto clamped_y = border_handler_.getY(py, height);

                                const auto pixel_offset = (static_cast<size_t>(clamped_y) * static_cast<size_t>(width) + static_cast<size_t>(clamped_x)) *
                                    static_cast<size_t>(channels) + static_cast<size_t>(c);
                                
                                // Масштабируем ядро на силу эффекта
                                const auto kernel_value = BASE_KERNEL[ky + 1][kx + 1] * strength;
                                sum += static_cast<double>(input_data[pixel_offset]) * kernel_value;
                            }
                        }

                        // Добавляем 128 для смещения в средний диапазон
                        // При strength = 0, результат будет близок к исходному изображению
                        const auto base_value = static_cast<double>(input_data[(static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * static_cast<size_t>(channels) + static_cast<size_t>(c)]);
                        const auto embossed_value = sum + 128.0;
                        // Интерполируем между исходным и обработанным значением в зависимости от strength
                        const auto value = base_value * (1.0 - strength) + embossed_value * strength;
                        const auto pixel_offset = (static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * static_cast<size_t>(channels) + static_cast<size_t>(c);
                        result[pixel_offset] = static_cast<uint8_t>(std::max(0.0, std::min(255.0, value)));
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

std::string EmbossFilter::getName() const
{
    return "emboss";
}

std::string EmbossFilter::getDescription() const
{
    return "Эффект рельефа";
}

std::string EmbossFilter::getCategory() const
{
    return "Края и детали";
}



