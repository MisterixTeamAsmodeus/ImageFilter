#include <filters/MedianFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/BorderHandler.h>
#include <utils/IBufferPool.h>
#include <utils/SafeMath.h>
#include <utils/FilterValidator.h>
#include <utils/FilterValidationHelper.h>
#include <algorithm>
#include <vector>
#include <cstring>

namespace
{

    /**
     * @brief Находит медиану из гистограммы
     * @param histogram Гистограмма значений [0-255]
     * @param total_count Общее количество элементов в окне
     * @return Медианное значение
     * 
     * Оптимизированная версия: использует ранний выход и оптимизированный цикл
     */
    [[nodiscard]] inline uint8_t findMedianFromHistogram(const int* histogram, int total_count) noexcept
    {
        // Для нечетного размера окна медиана - это элемент на позиции (total_count - 1) / 2
        // Для четного размера медиана - это среднее двух центральных элементов, но обычно берем меньший
        const int target = (total_count - 1) / 2;
        int count = 0;
        
        // Оптимизация: используем цикл с ранним выходом
        // Компилятор может оптимизировать этот цикл с помощью векторизации
        for (int i = 0; i < 256; ++i)
        {
            count += histogram[i];
            if (count > target)
            {
                return static_cast<uint8_t>(i);
            }
        }
        
        // Fallback: если медиана не найдена (не должно происходить), возвращаем последнее значение
        return 255;
    }
}

FilterResult MedianFilter::apply(ImageProcessor& image)
{
    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();

    // Валидация параметра фильтра
    auto radius_result = FilterValidator::validateRadius(radius_, 0, 1000, width, height);
    
    // Валидация изображения и параметра с автоматическим добавлением контекста
    auto validation_result = FilterValidationHelper::validateImageAndParam(
        image, radius_result, "radius", radius_);
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
        ctx.withFilterParam("radius", radius_);
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
    
    const auto window_size = (2 * radius_ + 1) * (2 * radius_ + 1);
    size_t row_stride = 0;
    if (!SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(channels), row_stride))
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        ctx.withFilterParam("radius", radius_);
        return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                   "Размер изображения слишком большой", ctx);
    }

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
                    const auto py = border_handler_.getY(y + ky, height);
                    const auto* row_base = input_data + static_cast<size_t>(py) * row_stride;

                    for (int kx = -radius_; kx <= radius_; ++kx)
                    {
                        const auto px = border_handler_.getX(kx, width);
                        const auto* pixel = row_base + static_cast<size_t>(px) * static_cast<size_t>(channels);
                        
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
                const bool is_center_x_region = (radius_ < width - radius_);

                // Обрабатываем остальные пиксели в строке, обновляя гистограмму
                for (int x = 1; x < width; ++x)
                {
                    // Удаляем левый столбец из гистограммы
                    const int left_x = x - radius_ - 1;
                    const int right_x = x + radius_;

                    if (is_center_region && is_center_x_region && x > radius_ && x < width - radius_)
                    {
                        // Самый быстрый путь: нет проверки границ ни по X, ни по Y
                        for (int ky = -radius_; ky <= radius_; ++ky)
                        {
                            const auto py = y + ky;
                            const auto* row_base = input_data + static_cast<size_t>(py) * row_stride;
                            
                            // В центре изображения left_x и right_x всегда валидны
                            const auto* left_pixel = row_base + static_cast<size_t>(left_x) * static_cast<size_t>(channels);
                            const auto* right_pixel = row_base + static_cast<size_t>(right_x) * static_cast<size_t>(channels);
                            
                            --histogram_r[left_pixel[0]];
                            --histogram_g[left_pixel[1]];
                            --histogram_b[left_pixel[2]];
                            
                            ++histogram_r[right_pixel[0]];
                            ++histogram_g[right_pixel[1]];
                            ++histogram_b[right_pixel[2]];
                        }
                    }
                    else if (is_center_region)
                    {
                        // Быстрый путь по Y: нет проверки границ по Y, но нужна проверка по X
                        for (int ky = -radius_; ky <= radius_; ++ky)
                        {
                            const auto py = y + ky;
                            const auto* row_base = input_data + static_cast<size_t>(py) * row_stride;
                            
                            if (left_x >= 0)
                            {
                                const auto* left_pixel = row_base + static_cast<size_t>(left_x) * static_cast<size_t>(channels);
                                --histogram_r[left_pixel[0]];
                                --histogram_g[left_pixel[1]];
                                --histogram_b[left_pixel[2]];
                            }
                            
                            if (right_x < width)
                            {
                                const auto* right_pixel = row_base + static_cast<size_t>(right_x) * static_cast<size_t>(channels);
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
                            const auto py = border_handler_.getY(y + ky, height);
                            const auto* row_base = input_data + static_cast<size_t>(py) * row_stride;
                            
                            const auto left_px = border_handler_.getX(left_x, width);
                            const auto right_px = border_handler_.getX(right_x, width);
                            
                            const auto* left_pixel = row_base + static_cast<size_t>(left_px) * static_cast<size_t>(channels);
                            const auto* right_pixel = row_base + static_cast<size_t>(right_px) * static_cast<size_t>(channels);
                            
                            --histogram_r[left_pixel[0]];
                            --histogram_g[left_pixel[1]];
                            --histogram_b[left_pixel[2]];
                            
                            ++histogram_r[right_pixel[0]];
                            ++histogram_g[right_pixel[1]];
                            ++histogram_b[right_pixel[2]];
                        }
                    }

                    // Вычисляем медиану для текущего пикселя
                    const auto pixel_offset = static_cast<size_t>(y) * row_stride + static_cast<size_t>(x) * static_cast<size_t>(channels);
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

    // Возвращаем буфер в пул для переиспользования
    if (buffer_pool_ != nullptr)
    {
        buffer_pool_->release(std::move(result));
    }

    return FilterResult::success();
}

std::string MedianFilter::getName() const
{
    return "median";
}

std::string MedianFilter::getDescription() const
{
    return "Медианный фильтр (удаление шума)";
}

std::string MedianFilter::getCategory() const
{
    return "Размытие и шум";
}

