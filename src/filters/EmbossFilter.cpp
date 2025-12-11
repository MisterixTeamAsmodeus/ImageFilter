#include <filters/EmbossFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/BorderHandler.h>
#include <algorithm>
#include <vector>

FilterResult EmbossFilter::apply(ImageProcessor& image)
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
        return FilterResult::failure(FilterError::InvalidSize,
                                     "Размер изображения должен быть больше нуля", ctx);
    }

    if (channels != 3 && channels != 4)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        return FilterResult::failure(FilterError::InvalidChannels, 
                                     "Ожидается 3 канала (RGB) или 4 канала (RGBA), получено: " + std::to_string(channels),
                                     ctx);
    }

    const auto* input_data = image.getData();
    const auto buffer_size = static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(channels);
    
    // Создаем буфер для результата
    std::vector<uint8_t> result(buffer_size);


    ParallelImageProcessor::processRowsParallel(
        height,
        width,
        [width, height, channels, input_data, &result, this](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    for (int c = 0; c < channels; ++c)
                    {
                        int sum = 0;

                        // Применяем ядро рельефа
                        for (int ky = -1; ky <= 1; ++ky)
                        {
                            for (int kx = -1; kx <= 1; ++kx)
                            {
                                // Ядро рельефа (emboss kernel)
                                constexpr int emboss_kernel[3][3] = {
                                    {-2, -1, 0},
                                    {-1, 1, 1},
                                    {0, 1, 2}
                                };
                                const auto px = x + kx;
                                const auto py = y + ky;

                                // Обработка границ с использованием BorderHandler
                                const auto clamped_x = border_handler_.getX(px, width);
                                const auto clamped_y = border_handler_.getY(py, height);

                                const auto pixel_offset = (static_cast<size_t>(clamped_y) * static_cast<size_t>(width) + static_cast<size_t>(clamped_x)) *
                                    static_cast<size_t>(channels) + static_cast<size_t>(c);
                                sum += static_cast<int>(input_data[pixel_offset]) * emboss_kernel[ky + 1][kx + 1];
                            }
                        }

                        // Добавляем 128 для смещения в средний диапазон
                        const auto value = sum + 128;
                        const auto pixel_offset = (static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * static_cast<size_t>(channels) + static_cast<size_t>(c);
                        result[pixel_offset] = static_cast<uint8_t>(std::max(0, std::min(255, value)));
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



