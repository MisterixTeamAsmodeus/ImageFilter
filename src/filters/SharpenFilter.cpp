#include <filters/SharpenFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/FilterValidator.h>
#include <utils/FilterValidationHelper.h>
#include <utils/BorderHandler.h>
#include <utils/IBufferPool.h>
#include <utils/SafeMath.h>
#include <algorithm>
#include <vector>
#include <cstdint>

FilterResult SharpenFilter::apply(ImageProcessor& image) {
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

    // Вычисляем ядро повышения резкости на основе параметра strength
    // Базовое ядро при strength = 1.0:
    //   0  -1   0
    //  -1   5  -1
    //   0  -1   0
    //
    // При strength != 1.0 ядро масштабируется:
    // - Центральный элемент: 1 + 4 * strength
    // - Соседние элементы: -strength
    //
    // Это позволяет плавно регулировать силу эффекта от 0 (без изменений) до любого значения
    constexpr int BASE_CENTER = 1;
    constexpr int BASE_NEIGHBOR = -1;
    constexpr int BASE_NEIGHBOR_COUNT = 4; // 4 соседних пикселя (верх, низ, лево, право)
    
    // Масштабируем для целочисленной арифметики (16 бит для точности)
    constexpr int SCALE = 65536;
    const int center_value = static_cast<int>((BASE_CENTER + BASE_NEIGHBOR_COUNT * strength_) * SCALE);
    const int neighbor_value = static_cast<int>(BASE_NEIGHBOR * strength_ * SCALE);

    // Создаем копию исходных данных для чтения
    // Это необходимо, так как мы будем изменять данные изображения,
    // но для вычисления новых значений нужны исходные значения соседних пикселей
    const auto* input_data = image.getData();
    size_t width_height_product = 0;
    size_t image_size = 0;
    if (!SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), width_height_product) ||
        !SafeMath::safeMultiply(width_height_product, static_cast<size_t>(channels), image_size))
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        ctx.withFilterParam("strength", strength_);
        return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                   "Размер изображения слишком большой", ctx);
    }
    
    // Получаем буфер из пула или создаем новый
    std::vector<uint8_t> input_copy;
    if (buffer_pool_ != nullptr)
    {
        input_copy = buffer_pool_->acquire(image_size);
        // Копируем данные в буфер
        std::copy(input_data, input_data + image_size, input_copy.begin());
    }
    else
    {
        input_copy.assign(input_data, input_data + image_size);
    }

    auto* output_data = image.getData();

    // Параллельная обработка строк изображения
    ParallelImageProcessor::processRowsParallel(
        height,
        width,
        [width, height, channels, &input_copy, output_data, center_value, neighbor_value, this](int start_row, int end_row) {
            // Обрабатываем строки в диапазоне [start_row, end_row)
            for (int y = start_row; y < end_row; ++y) {
                const auto row_offset = static_cast<size_t>(y) * static_cast<size_t>(width) * static_cast<size_t>(channels);
                
                for (int x = 0; x < width; ++x) {
                    // Применяем ядро 3x3 с динамическими коэффициентами
                    constexpr int kernel_size = 3;
                    constexpr int kernel_radius = kernel_size / 2; // = 1
                    
                    // Обрабатываем каждый канал отдельно
                    for (int c = 0; c < channels; ++c) {
                        int64_t sum = 0;

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
                                const auto pixel_value = static_cast<int64_t>(input_copy[pixel_index]);
                                
                                // Определяем коэффициент ядра для текущей позиции
                                int kernel_coeff;
                                if (ky == kernel_radius && kx == kernel_radius) {
                                    // Центральный пиксель
                                    kernel_coeff = center_value;
                                } else if ((ky == kernel_radius && (kx == kernel_radius - 1 || kx == kernel_radius + 1)) ||
                                          (kx == kernel_radius && (ky == kernel_radius - 1 || ky == kernel_radius + 1))) {
                                    // Соседние пиксели (верх, низ, лево, право)
                                    kernel_coeff = neighbor_value;
                                } else {
                                    // Угловые пиксели (не используются в базовом ядре)
                                    kernel_coeff = 0;
                                }
                                
                                constexpr int SCALE = 65536;
                                sum += (pixel_value * kernel_coeff) / SCALE;
                            }
                        }

                        // Ограничиваем значение диапазоном [0, 255]
                        // Это предотвращает переполнение и отрицательные значения
                        const auto clamped_sum = std::max(0LL, std::min(255LL, sum));

                        const auto result_index = row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels) + static_cast<size_t>(c);
                        output_data[result_index] = static_cast<uint8_t>(clamped_sum);
                    }
                }
            }
        }
    );

    // Возвращаем буфер в пул для переиспользования
    if (buffer_pool_ != nullptr)
    {
        buffer_pool_->release(std::move(input_copy));
    }

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


