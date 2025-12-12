#include <filters/MedianFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <algorithm>
#include <vector>
#include <cstring>

namespace
{
    /**
     * @brief Быстрое отражение координаты для обработки границ
     * @param coord Координата для отражения
     * @param max Максимальное значение координаты (не включительно)
     * @return Отраженная координата в диапазоне [0, max)
     */
    inline int reflectCoordinate(int coord, int max) noexcept
    {
        if (coord < 0)
        {
            return -coord;
        }
        if (coord >= max)
        {
            return 2 * max - coord - 1;
        }
        return coord;
    }

    /**
     * @brief Находит медиану из гистограммы
     * @param histogram Гистограмма значений [0-255]
     * @param total_count Общее количество элементов в окне
     * @return Медианное значение
     */
    inline uint8_t findMedianFromHistogram(const int* histogram, int total_count) noexcept
    {
        const auto target = total_count / 2;
        int count = 0;
        
        for (int i = 0; i < 256; ++i)
        {
            count += histogram[i];
            if (count > target)
            {
                return static_cast<uint8_t>(i);
            }
        }
        
        return 255;
    }
}

bool MedianFilter::apply(ImageProcessor& image)
{
    if (!image.isValid())
    {
        return false;
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();

    if (channels != 3 || radius_ <= 0)
    {
        return false;
    }

    const auto* input_data = image.getData();
    std::vector<uint8_t> result(static_cast<size_t>(width) * height * channels);
    
    const auto window_size = (2 * radius_ + 1) * (2 * radius_ + 1);
    const auto row_stride = width * channels;

    ParallelImageProcessor::processRowsParallel(
        height,
        [width, height, channels, input_data, &result, this, window_size, row_stride](
            int start_row, int end_row)
        {
            // Гистограммы для каждого канала (256 значений для uint8_t)
            int histogram_r[256];
            int histogram_g[256];
            int histogram_b[256];

            for (int y = start_row; y < end_row; ++y)
            {
                // Инициализируем гистограммы для первого окна в строке
                std::memset(histogram_r, 0, sizeof(histogram_r));
                std::memset(histogram_g, 0, sizeof(histogram_g));
                std::memset(histogram_b, 0, sizeof(histogram_b));

                // Заполняем гистограмму для первого окна (x=0)
                for (int ky = -radius_; ky <= radius_; ++ky)
                {
                    const auto py = reflectCoordinate(y + ky, height);
                    const auto* row_base = input_data + static_cast<size_t>(py) * row_stride;

                    for (int kx = -radius_; kx <= radius_; ++kx)
                    {
                        const auto px = reflectCoordinate(kx, width);
                        const auto* pixel = row_base + px * channels;
                        
                        ++histogram_r[pixel[0]];
                        ++histogram_g[pixel[1]];
                        ++histogram_b[pixel[2]];
                    }
                }

                // Вычисляем медиану для первого пикселя
                result[static_cast<size_t>(y) * row_stride + 0] = 
                    findMedianFromHistogram(histogram_r, window_size);
                result[static_cast<size_t>(y) * row_stride + 1] = 
                    findMedianFromHistogram(histogram_g, window_size);
                result[static_cast<size_t>(y) * row_stride + 2] = 
                    findMedianFromHistogram(histogram_b, window_size);

                // Оптимизация: проверяем, находимся ли мы в центре изображения
                const bool is_center_region = (y >= radius_ && y < height - radius_);

                // Обрабатываем остальные пиксели в строке, обновляя гистограмму
                for (int x = 1; x < width; ++x)
                {
                    // Удаляем левый столбец из гистограммы
                    const int left_x = x - radius_ - 1;
                    const int right_x = x + radius_;

                    if (is_center_region)
                    {
                        // Быстрый путь: нет проверки границ
                        for (int ky = -radius_; ky <= radius_; ++ky)
                        {
                            const auto py = y + ky;
                            const auto* row_base = input_data + static_cast<size_t>(py) * row_stride;
                            
                            if (left_x >= 0)
                            {
                                const auto* left_pixel = row_base + left_x * channels;
                                --histogram_r[left_pixel[0]];
                                --histogram_g[left_pixel[1]];
                                --histogram_b[left_pixel[2]];
                            }
                            
                            if (right_x < width)
                            {
                                const auto* right_pixel = row_base + right_x * channels;
                                ++histogram_r[right_pixel[0]];
                                ++histogram_g[right_pixel[1]];
                                ++histogram_b[right_pixel[2]];
                            }
                        }
                    }
                    else
                    {
                        // Медленный путь: обработка границ
                        for (int ky = -radius_; ky <= radius_; ++ky)
                        {
                            const auto py = reflectCoordinate(y + ky, height);
                            const auto* row_base = input_data + static_cast<size_t>(py) * row_stride;
                            
                            const auto left_px = reflectCoordinate(left_x, width);
                            const auto right_px = reflectCoordinate(right_x, width);
                            
                            const auto* left_pixel = row_base + left_px * channels;
                            const auto* right_pixel = row_base + right_px * channels;
                            
                            --histogram_r[left_pixel[0]];
                            --histogram_g[left_pixel[1]];
                            --histogram_b[left_pixel[2]];
                            
                            ++histogram_r[right_pixel[0]];
                            ++histogram_g[right_pixel[1]];
                            ++histogram_b[right_pixel[2]];
                        }
                    }

                    // Вычисляем медиану для текущего пикселя
                    const auto pixel_offset = static_cast<size_t>(y) * row_stride + x * channels;
                    result[pixel_offset + 0] = findMedianFromHistogram(histogram_r, window_size);
                    result[pixel_offset + 1] = findMedianFromHistogram(histogram_g, window_size);
                    result[pixel_offset + 2] = findMedianFromHistogram(histogram_b, window_size);
                }
            }
        }
    );

    // Копируем результат обратно
    auto* data = image.getData();
    std::ranges::copy(result, data);

    return true;
}
