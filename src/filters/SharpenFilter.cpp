#include <filters/SharpenFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/BorderHandler.h>
#include <algorithm>
#include <vector>
#include <cstdint>

FilterResult SharpenFilter::apply(ImageProcessor& image) {
    if (!image.isValid()) {
        return FilterResult::failure(FilterError::InvalidImage, "Изображение не загружено");
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();

    // Валидация размеров изображения
    if (width <= 0 || height <= 0) {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        return FilterResult::failure(FilterError::InvalidSize,
                                     "Размер изображения должен быть больше нуля", ctx);
    }

    if (channels != 3 && channels != 4) {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        return FilterResult::failure(FilterError::InvalidChannels, 
                                     "Ожидается 3 канала (RGB) или 4 канала (RGBA), получено: " + std::to_string(channels),
                                     ctx);
    }

    // Ядро повышения резкости (3x3) в целочисленном формате
    // Это классическое ядро unsharp masking:
    //   0  -1   0
    //  -1   5  -1
    //   0  -1   0
    //
    // Центральный пиксель умножается на 5, а соседние по вертикали и горизонтали
    // вычитаются. Это усиливает края и детали изображения.

    // Создаем копию исходных данных для чтения
    // Это необходимо, так как мы будем изменять данные изображения,
    // но для вычисления новых значений нужны исходные значения соседних пикселей
    const auto* input_data = image.getData();
    const auto image_size = static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(channels);
    
    // Создаем буфер для копии входных данных
    std::vector<uint8_t> input_copy(input_data, input_data + image_size);

    auto* output_data = image.getData();

    // Параллельная обработка строк изображения
    ParallelImageProcessor::processRowsParallel(
        height,
        width,
        [width, height, channels, &input_copy, output_data, this](int start_row, int end_row) {
            // Обрабатываем строки в диапазоне [start_row, end_row)
            for (int y = start_row; y < end_row; ++y) {
                const auto row_offset = static_cast<size_t>(y) * static_cast<size_t>(width) * static_cast<size_t>(channels);
                
                for (int x = 0; x < width; ++x) {
                    // Применяем ядро 3x3
                    constexpr int kernel_size = 3;
                    constexpr int kernel_radius = kernel_size / 2; // = 1
                    constexpr int kernel[3][3] = {
                        {  0, -1,  0},
                        { -1,  5, -1},
                        {  0, -1,  0}
                    };
                    
                    // Обрабатываем каждый канал отдельно
                    for (int c = 0; c < channels; ++c) {
                        int sum = 0;

                        for (int ky = 0; ky < kernel_size; ++ky) {
                            const auto sample_y = y + ky - kernel_radius;
                            
                            // Обработка границ с использованием BorderHandler
                            const auto clamped_y = border_handler_.getY(sample_y, height);
                            
                            const auto row_offset_y = static_cast<size_t>(clamped_y) * static_cast<size_t>(width) * static_cast<size_t>(channels);
                            
                            for (int kx = 0; kx < kernel_size; ++kx) {
                                const auto sample_x = x + kx - kernel_radius;

                                // Обработка границ с использованием BorderHandler
                                const auto clamped_x = border_handler_.getX(sample_x, width);

                                const auto pixel_index = row_offset_y + static_cast<size_t>(clamped_x) * static_cast<size_t>(channels) + static_cast<size_t>(c);
                                sum += static_cast<int>(input_copy[pixel_index]) * kernel[ky][kx];
                            }
                        }

                        // Ограничиваем значение диапазоном [0, 255]
                        // Это предотвращает переполнение и отрицательные значения
                        sum = std::max(0, std::min(255, sum));

                        const auto result_index = row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels) + static_cast<size_t>(c);
                        output_data[result_index] = static_cast<uint8_t>(sum);
                    }
                }
            }
        }
    );

    return FilterResult::success();
}

std::string SharpenFilter::getName() const
{
    return "sharpen";
}

std::string SharpenFilter::getDescription() const
{
    return "Повышение резкости";
}

std::string SharpenFilter::getCategory() const
{
    return "Края и детали";
}


