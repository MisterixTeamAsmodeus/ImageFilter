#include <filters/SharpenFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <algorithm>
#include <vector>
#include <cstdint>

bool SharpenFilter::apply(ImageProcessor& image) {
    if (!image.isValid()) {
        return false;
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();

    if (channels != 3) {
        return false;
    }

    // Ядро повышения резкости (3x3) в целочисленном формате
    // Это классическое ядро unsharp masking:
    //   0  -1   0
    //  -1   5  -1
    //   0  -1   0
    //
    // Центральный пиксель умножается на 5, а соседние по вертикали и горизонтали
    // вычитаются. Это усиливает края и детали изображения.
    constexpr int kernel_size = 3;
    constexpr int kernel[kernel_size][kernel_size] = {
        {  0, -1,  0},
        { -1,  5, -1},
        {  0, -1,  0}
    };

    constexpr auto kernel_radius = kernel_size / 2; // = 1

    // Создаем копию исходных данных для чтения
    // Это необходимо, так как мы будем изменять данные изображения,
    // но для вычисления новых значений нужны исходные значения соседних пикселей
    const auto* input_data = image.getData();
    const auto image_size = static_cast<size_t>(width) * height * channels;
    auto input_copy = std::vector<uint8_t>(input_data, input_data + image_size);

    auto* output_data = image.getData();

    // Параллельная обработка строк изображения
    ParallelImageProcessor::processRowsParallel(
        height,
        [width, height, channels, kernel_size, kernel_radius, &input_copy, output_data, &kernel](int start_row, int end_row) {
            // Обрабатываем строки в диапазоне [start_row, end_row)
            for (int y = start_row; y < end_row; ++y) {
                const auto row_offset = static_cast<size_t>(y) * width * channels;
                
                for (int x = 0; x < width; ++x) {
                    // Обрабатываем каждый канал отдельно
                    for (int c = 0; c < channels; ++c) {
                        int sum = 0;

                        // Применяем ядро 3x3
                        for (int ky = 0; ky < kernel_size; ++ky) {
                            const auto sample_y = y + ky - kernel_radius;
                            
                            // Обработка границ: используем отражение
                            int clamped_y = sample_y;
                            if (clamped_y < 0) {
                                clamped_y = -clamped_y;
                            } else if (clamped_y >= height) {
                                clamped_y = 2 * height - clamped_y - 1;
                            }
                            
                            const auto row_offset_y = static_cast<size_t>(clamped_y) * width * channels;
                            
                            for (int kx = 0; kx < kernel_size; ++kx) {
                                const auto sample_x = x + kx - kernel_radius;

                                // Обработка границ: используем отражение
                                int clamped_x = sample_x;
                                if (clamped_x < 0) {
                                    clamped_x = -clamped_x;
                                } else if (clamped_x >= width) {
                                    clamped_x = 2 * width - clamped_x - 1;
                                }

                                const auto pixel_index = row_offset_y + static_cast<size_t>(clamped_x) * channels + c;
                                sum += static_cast<int>(input_copy[pixel_index]) * kernel[ky][kx];
                            }
                        }

                        // Ограничиваем значение диапазоном [0, 255]
                        // Это предотвращает переполнение и отрицательные значения
                        sum = std::max(0, std::min(255, sum));

                        const auto result_index = row_offset + static_cast<size_t>(x) * channels + c;
                        output_data[result_index] = static_cast<uint8_t>(sum);
                    }
                }
            }
        }
    );

    return true;
}

