#include <filters/GrayscaleFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/FilterValidationHelper.h>
#include <utils/ColorConversionUtils.h>

FilterResult GrayscaleFilter::apply(ImageProcessor& image)
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

    // Параллельная обработка строк изображения
    // Каждый поток обрабатывает свой диапазон строк [start_row, end_row)
    ParallelImageProcessor::processRowsParallel(
        height,
        [width, channels, data](int start_row, int end_row)
        {
            // Обрабатываем строки в диапазоне [start_row, end_row)
            for (int y = start_row; y < end_row; ++y)
            {
                const auto row_offset = static_cast<size_t>(y) * static_cast<size_t>(width) * static_cast<size_t>(channels);

                // Обрабатываем каждый пиксель в строке
                for (int x = 0; x < width; ++x)
                {
                    const auto pixel_offset = row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels);

                    // Получаем значения каналов RGB
                    const auto r = static_cast<int>(data[pixel_offset + 0]);
                    const auto g = static_cast<int>(data[pixel_offset + 1]);
                    const auto b = static_cast<int>(data[pixel_offset + 2]);

                    // Применяем формулу преобразования в градации серого
                    // Используем общую утилиту для устранения дублирования кода
                    const auto gray = ColorConversionUtils::rgbToGrayscale(r, g, b);

                    // Присваиваем одинаковое значение всем трем цветовым каналам
                    // Альфа-канал (если есть) сохраняется без изменений
                    data[pixel_offset + 0] = gray; // R
                    data[pixel_offset + 1] = gray; // G
                    data[pixel_offset + 2] = gray; // B
                    // Альфа-канал (pixel_offset + 3) не изменяется, если channels == 4
                }
            }
        }
    );

    return FilterResult::success();
}

std::string GrayscaleFilter::getName() const
{
    return "grayscale";
}

std::string GrayscaleFilter::getDescription() const
{
    return "Преобразование в оттенки серого";
}

std::string GrayscaleFilter::getCategory() const
{
    return "Цветовой";
}

bool GrayscaleFilter::supportsInPlace() const noexcept
{
    return true;
}

