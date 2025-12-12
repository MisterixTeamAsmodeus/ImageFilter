#include <filters/GrayscaleFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>

bool GrayscaleFilter::apply(ImageProcessor& image)
{
    if (!image.isValid())
    {
        return false;
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();

    if (channels != 3)
    {
        // Фильтр работает только с RGB изображениями
        return false;
    }

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
                const auto row_offset = static_cast<size_t>(y) * width * channels;

                // Обрабатываем каждый пиксель в строке
                for (int x = 0; x < width; ++x)
                {
                    // Константы для преобразования RGB в градации серого
                    // Эти коэффициенты основаны на восприятии яркости человеческим глазом:
                    // - Зеленый канал имеет наибольший вес (0.587), так как глаз наиболее чувствителен к зеленому
                    // - Красный канал имеет средний вес (0.299)
                    // - Синий канал имеет наименьший вес (0.114)
                    // Используем целочисленную арифметику для оптимизации:
                    // Y = (299*R + 587*G + 114*B) / 1000
                    // Для точности используем: (19595*R + 38470*G + 7471*B) >> 16
                    // Это эквивалентно формуле с коэффициентами 0.299, 0.587, 0.114
                    constexpr int R_COEFF = 19595; // 0.299 * 65536
                    constexpr int G_COEFF = 38470; // 0.587 * 65536
                    constexpr int B_COEFF = 7471; // 0.114 * 65536

                    const auto pixel_offset = row_offset + static_cast<size_t>(x) * channels;

                    // Получаем значения каналов
                    const auto r = static_cast<int>(data[pixel_offset + 0]);
                    const auto g = static_cast<int>(data[pixel_offset + 1]);
                    const auto b = static_cast<int>(data[pixel_offset + 2]);

                    // Применяем формулу преобразования в градации серого
                    // Используем целочисленную арифметику с битовым сдвигом для деления на 65536
                    // Это быстрее, чем операции с плавающей точкой
                    const auto gray = static_cast<uint8_t>((R_COEFF * r + G_COEFF * g + B_COEFF * b) >> 16);

                    // Присваиваем одинаковое значение всем трем каналам
                    // Это сохраняет формат RGB, но все каналы имеют одинаковое значение
                    data[pixel_offset + 0] = gray; // R
                    data[pixel_offset + 1] = gray; // G
                    data[pixel_offset + 2] = gray; // B
                }
            }
        }
    );

    return true;
}
