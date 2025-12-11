#include <filters/GaussianBlurFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/BorderHandler.h>
#include <utils/LookupTables.h>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <shared_mutex>
#include <numbers>  // C++20: для std::numbers::pi

// Масштаб для целочисленной арифметики (16 бит для точности)
constexpr int32_t KERNEL_SCALE = 65536;

namespace {
    /**
     * @brief Получает thread-safe кэш для ядер Гаусса
     * Использует локальную статическую переменную для избежания exit-time destructors
     * @return Ссылка на кэш ядер
     */
    std::unordered_map<double, std::vector<int32_t>>& getKernelCache()
    {
        static std::unordered_map<double, std::vector<int32_t>> cache;
        return cache;
    }

    /**
     * @brief Получает мьютекс для синхронизации доступа к кэшу
     * Использует локальную статическую переменную для избежания exit-time destructors
     * @return Ссылка на мьютекс кэша
     */
    std::shared_mutex& getCacheMutex()
    {
        static std::shared_mutex mutex;
        return mutex;
    }

    /**
     * @brief Генерирует одномерное ядро Гаусса в целочисленном формате
     * @param radius Радиус размытия
     * @param sigma Стандартное отклонение (вычисляется из radius)
     * @return Вектор коэффициентов ядра (масштабированные на 65536 для точности)
     */
    std::vector<int32_t> generateKernel(const double radius, const double& sigma)
    {
        // Инициализируем lookup tables при первом использовании
        LookupTables::initialize();

        // Размер ядра должен быть нечетным и покрывать 3 сигмы в каждую сторону
        // Это обеспечивает, что 99.7% веса функции Гаусса попадет в ядро
        const auto kernel_size = static_cast<int>(std::ceil(radius * 2.0)) | 1; // Делаем нечетным
        const auto center = kernel_size / 2;

        auto kernel = std::vector<int32_t>(static_cast<size_t>(kernel_size));
        const auto two_sigma_squared = 2.0 * sigma * sigma;
        // Используем std::numbers::pi из C++20 вместо M_PI для переносимости
        const auto sqrt_two_pi_sigma = std::sqrt(2.0 * std::numbers::pi) * sigma;

        // Генерируем значения функции Гаусса для каждого элемента ядра
        // Масштабируем на KERNEL_SCALE для целочисленной арифметики
        // Используем lookup table для exp вместо std::exp для оптимизации
        for (int i = 0; i < kernel_size; ++i)
        {
            const auto distance = static_cast<double>(i - center);
            // Формула функции Гаусса: G(x) = (1 / (σ * √(2π))) * e^(-(x² / (2σ²))))
            const auto exponent = (distance * distance) / two_sigma_squared;
            const auto value = LookupTables::expNegative(exponent) / sqrt_two_pi_sigma;
            kernel[static_cast<size_t>(i)] = static_cast<int32_t>(value * KERNEL_SCALE);
        }

        return kernel;
    }

    /**
     * @brief Нормализует ядро (сумма коэффициентов = масштаб)
     * @param kernel Ядро для нормализации
     */
    void normalizeKernel(std::vector<int32_t>& kernel)
    {
        // Вычисляем сумму всех коэффициентов ядра
        int64_t sum = 0;
        for (const int32_t value : kernel)
        {
            sum += value;
        }

        // Нормализуем ядро так, чтобы сумма коэффициентов равнялась KERNEL_SCALE
        // Это гарантирует, что яркость изображения не изменится после применения фильтра
        if (sum > 0)
        {
            for (int32_t& value : kernel)
            {
                value = static_cast<int32_t>((static_cast<int64_t>(value) * KERNEL_SCALE) / sum);
            }
        }
    }

    /**
     * @brief Генерирует или получает из кэша одномерное ядро Гаусса
     * @param radius Радиус размытия
     * @param sigma Стандартное отклонение (вычисляется из radius)
     * @return Вектор коэффициентов ядра (масштабированные на 65536 для точности)
     */
    std::vector<int32_t> getOrGenerateKernel(const double radius, const double& sigma)
    {
        auto& kernel_cache = getKernelCache();
        auto& cache_mutex = getCacheMutex();

        // Пытаемся получить ядро из кэша (shared lock для чтения)
        {
            std::shared_lock<std::shared_mutex> lock(cache_mutex);
            const auto it = kernel_cache.find(radius);
            if (it != kernel_cache.end())
            {
                return it->second;  // Возвращаем копию из кэша
            }
        }

        // Ядро не найдено в кэше, генерируем новое
        auto kernel = generateKernel(radius, sigma);
        normalizeKernel(kernel);

        // Сохраняем в кэш (exclusive lock для записи)
        {
            std::unique_lock<std::shared_mutex> lock(cache_mutex);
            // Проверяем еще раз на случай, если другой поток уже добавил ядро
            if (kernel_cache.find(radius) == kernel_cache.end())
            {
                kernel_cache[radius] = kernel;
            }
            else
            {
                // Используем уже существующее ядро из кэша
                kernel = kernel_cache[radius];
            }
        }

        return kernel;
    }

    /**
     * @brief Применяет одномерное ядро по горизонтали
     * @param image Исходное изображение
     * @param kernel Ядро для применения (целочисленное, масштабированное)
     * @param border_handler Обработчик границ
     * @return Вектор с промежуточными результатами
     */
    std::vector<uint8_t> applyHorizontalKernel(
        const ImageProcessor& image,
        const std::vector<int32_t>& kernel,
        const BorderHandler& border_handler
    )
    {
        const auto width = image.getWidth();
        const auto height = image.getHeight();
        const auto channels = image.getChannels();
        const auto kernel_size = static_cast<int>(kernel.size());
        const auto kernel_radius = kernel_size / 2;

        const auto* input_data = image.getData();
        const auto buffer_size = static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(channels);
        
        // Создаем буфер для результата
        std::vector<uint8_t> result(buffer_size);

        // Параллельная обработка строк изображения
        ParallelImageProcessor::processRowsParallel(
            height,
            [input_data, &result, &kernel, &border_handler, width, channels, kernel_size, kernel_radius](int start_row, int end_row)
            {
                // Обрабатываем строки в диапазоне [start_row, end_row)
                for (int y = start_row; y < end_row; ++y)
                {
                    const auto row_offset = static_cast<size_t>(y) * static_cast<size_t>(width) * static_cast<size_t>(channels);

                    for (int x = 0; x < width; ++x)
                    {
                        // Обрабатываем каждый канал отдельно
                        for (int c = 0; c < channels; ++c)
                        {
                            int64_t sum = 0;

                            // Применяем ядро по горизонтали
                            for (int k = 0; k < kernel_size; ++k)
                            {
                                const auto sample_x = x + k - kernel_radius;

                                // Обработка границ с использованием BorderHandler
                                const auto clamped_x = border_handler.getX(sample_x, width);

                                const auto pixel_index = row_offset + static_cast<size_t>(clamped_x) * static_cast<size_t>(channels) + static_cast<size_t>(c);
                                sum += static_cast<int64_t>(input_data[pixel_index]) * kernel[static_cast<size_t>(k)];
                            }

                            // Деление на масштаб с округлением
                            // KERNEL_SCALE всегда > 0, но добавляем проверку для безопасности
                            if (KERNEL_SCALE > 0)
                            {
                                // Защита от переполнения: проверяем, что sum не слишком большой
                                const int64_t max_safe_sum = static_cast<int64_t>(INT_MAX) * KERNEL_SCALE;
                                if (sum > max_safe_sum)
                                {
                                    sum = max_safe_sum;
                                }
                                else if (sum < -max_safe_sum)
                                {
                                    sum = -max_safe_sum;
                                }
                                
                                const auto result_value = static_cast<int>((sum + (KERNEL_SCALE / 2)) / KERNEL_SCALE);
                                const auto result_index = row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels) + static_cast<size_t>(c);
                                result[result_index] = static_cast<uint8_t>(std::max(0, std::min(255, result_value)));
                            }
                            else
                            {
                                // Fallback: используем исходное значение
                                const auto result_index = row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels) + static_cast<size_t>(c);
                                result[result_index] = input_data[result_index];
                            }
                        }
                    }
                }
            }
        );

        return result;
    }

    /**
     * @brief Применяет одномерное ядро по вертикали
     * @param horizontalResult Результат горизонтального применения
     * @param image Исходное изображение (для получения размеров)
     * @param kernel Ядро для применения (целочисленное, масштабированное)
     * @param border_handler Обработчик границ
     * @return Финальный результат размытия
     */
    std::vector<uint8_t> applyVerticalKernel(
        const std::vector<uint8_t>& horizontalResult,
        const ImageProcessor& image,
        const std::vector<int32_t>& kernel,
        const BorderHandler& border_handler
    )
    {
        const auto width = image.getWidth();
        const auto height = image.getHeight();
        const auto channels = image.getChannels();
        const auto kernel_size = static_cast<int>(kernel.size());
        const auto kernel_radius = kernel_size / 2;

        const auto buffer_size = static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(channels);
        
        // Создаем буфер для результата
        std::vector<uint8_t> result(buffer_size);

        // Параллельная обработка строк изображения
        ParallelImageProcessor::processRowsParallel(
            height,
            [&horizontalResult, &result, &kernel, &border_handler, width, height, channels, kernel_size, kernel_radius](
            int start_row, int end_row)
            {
                // Обрабатываем строки в диапазоне [start_row, end_row)
                for (int y = start_row; y < end_row; ++y)
                {
                    const auto row_offset = static_cast<size_t>(y) * static_cast<size_t>(width) * static_cast<size_t>(channels);

                    for (int x = 0; x < width; ++x)
                    {
                        // Обрабатываем каждый канал отдельно
                        for (int c = 0; c < channels; ++c)
                        {
                            int64_t sum = 0;

                            // Применяем ядро по вертикали
                            for (int k = 0; k < kernel_size; ++k)
                            {
                                const auto sample_y = y + k - kernel_radius;

                                // Обработка границ с использованием BorderHandler
                                const auto clamped_y = border_handler.getY(sample_y, height);

                                const auto pixel_index = (static_cast<size_t>(clamped_y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * static_cast<size_t>(channels) + static_cast<size_t>(c);
                                sum += static_cast<int64_t>(horizontalResult[pixel_index]) * kernel[static_cast<size_t>(k)];
                            }

                            // Деление на масштаб с округлением
                            // KERNEL_SCALE всегда > 0, но добавляем проверку для безопасности
                            if (KERNEL_SCALE > 0)
                            {
                                // Защита от переполнения: проверяем, что sum не слишком большой
                                const int64_t max_safe_sum = static_cast<int64_t>(INT_MAX) * KERNEL_SCALE;
                                if (sum > max_safe_sum)
                                {
                                    sum = max_safe_sum;
                                }
                                else if (sum < -max_safe_sum)
                                {
                                    sum = -max_safe_sum;
                                }
                                
                                const auto result_value = static_cast<int>((sum + (KERNEL_SCALE / 2)) / KERNEL_SCALE);
                                const auto result_index = row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels) + static_cast<size_t>(c);
                                result[result_index] = static_cast<uint8_t>(std::max(0, std::min(255, result_value)));
                            }
                            else
                            {
                                // Fallback: используем исходное значение
                                const auto result_index = row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels) + static_cast<size_t>(c);
                                result[result_index] = horizontalResult[result_index];
                            }
                        }
                    }
                }
            }
        );

        return result;
    }
}

FilterResult GaussianBlurFilter::apply(ImageProcessor& image)
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
        ctx.filter_params = "radius=" + std::to_string(radius_);
        return FilterResult::failure(FilterError::InvalidSize,
                                     "Размер изображения должен быть больше нуля", ctx);
    }

    if (channels != 3 && channels != 4)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        ctx.filter_params = "radius=" + std::to_string(radius_);
        return FilterResult::failure(FilterError::InvalidChannels, 
                                     "Ожидается 3 канала (RGB) или 4 канала (RGBA), получено: " + std::to_string(channels),
                                     ctx);
    }

    // Валидация параметра фильтра
    if (radius_ <= 0.0)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        ctx.filter_params = "radius=" + std::to_string(radius_);
        return FilterResult::failure(FilterError::InvalidRadius,
                                     "Радиус должен быть больше нуля, получено: " + std::to_string(radius_),
                                     ctx);
    }

    // Вычисляем стандартное отклонение (sigma) из радиуса
    // Эмпирическое правило: sigma ≈ radius / 2
    auto sigma = radius_ / 2.0;

    // Получаем ядро из кэша или генерируем новое
    auto kernel = getOrGenerateKernel(radius_, sigma);

    // Применяем separable kernel: сначала по горизонтали, затем по вертикали
    // Это оптимизация: вместо O(N²) операций на пиксель получаем O(2N)
    // Примечание: applyHorizontalKernel и applyVerticalKernel создают свои локальные пулы
    
    auto horizontal_result = applyHorizontalKernel(image, kernel, border_handler_);
    auto final_result = applyVerticalKernel(horizontal_result, image, kernel, border_handler_);

    // Копируем результат обратно в изображение
    auto* data = image.getData();
    std::ranges::copy(final_result, data);

    // Буферы автоматически освобождаются при выходе из области видимости
    // (applyHorizontalKernel и applyVerticalKernel создают свои локальные пулы)

    return FilterResult::success();
}


std::string GaussianBlurFilter::getName() const
{
    return "blur";
}

std::string GaussianBlurFilter::getDescription() const
{
    return "Размытие по Гауссу";
}

std::string GaussianBlurFilter::getCategory() const
{
    return "Размытие и шум";
}

