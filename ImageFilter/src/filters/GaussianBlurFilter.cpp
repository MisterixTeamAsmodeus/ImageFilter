#include <filters/GaussianBlurFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/BorderHandler.h>
#include <utils/LookupTables.h>
#include <utils/IBufferPool.h>
#include <utils/CacheManager.h>
#include <utils/SafeMath.h>
#include <utils/FilterValidator.h>
#include <utils/FilterValidationHelper.h>
#include <algorithm>
#include <vector>
#include <cmath>
#include <numbers>  // C++20: для std::numbers::pi

// Масштаб для целочисленной арифметики (16 бит для точности)
constexpr int32_t KERNEL_SCALE = 65536;

namespace {
    /**
     * @brief Генерирует одномерное ядро Гаусса в целочисленном формате
     * @param radius Радиус размытия
     * @param sigma Стандартное отклонение (вычисляется из radius)
     * @return Вектор коэффициентов ядра (масштабированные на 65536 для точности)
     */
    std::vector<int32_t> generateKernel(const double radius, const double &sigma) {
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
        for (int i = 0; i < kernel_size; ++i) {
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
    void normalizeKernel(std::vector<int32_t> &kernel) {
        // Вычисляем сумму всех коэффициентов ядра
        int64_t sum = 0;
        for (const int32_t value: kernel) {
            sum += value;
        }

        // Нормализуем ядро так, чтобы сумма коэффициентов равнялась KERNEL_SCALE
        // Это гарантирует, что яркость изображения не изменится после применения фильтра
        if (sum > 0) {
            for (int32_t &value: kernel) {
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
    std::vector<int32_t> getOrGenerateKernel(const double radius, const double &sigma) {
        KernelCacheKey key{};
        key.type = KernelCacheKey::Type::Gaussian;
        key.radius = radius;
        key.sigma = sigma;

        auto &cache_manager = CacheManager::getInstance();
        return cache_manager.getOrGenerateKernel(key, [radius, sigma]() {
            auto kernel = generateKernel(radius, sigma);
            normalizeKernel(kernel);
            return kernel;
        });
    }

    /**
     * @brief Применяет одномерное ядро по горизонтали
     * @param image Исходное изображение
     * @param kernel Ядро для применения (целочисленное, масштабированное)
     * @param border_handler Обработчик границ
     * @param buffer_pool Пул буферов для переиспользования (может быть nullptr)
     * @return Вектор с промежуточными результатами
     */
    std::vector<uint8_t> applyHorizontalKernel(
        const ImageProcessor &image,
        const std::vector<int32_t> &kernel,
        const BorderHandler &border_handler,
        IBufferPool *buffer_pool
    ) {
        const auto width = image.getWidth();
        const auto height = image.getHeight();
        const auto channels = image.getChannels();
        const auto kernel_size = static_cast<int>(kernel.size());
        const auto kernel_radius = kernel_size / 2;

        const auto *input_data = image.getData();
        size_t width_height_product = 0;
        size_t buffer_size = 0;
        if (!SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), width_height_product) ||
            !SafeMath::safeMultiply(width_height_product, static_cast<size_t>(channels), buffer_size)) {
            // Возвращаем пустой вектор при переполнении
            return std::vector<uint8_t>();
        }

        // Получаем буфер из пула или создаем новый
        std::vector<uint8_t> result;
        if (buffer_pool != nullptr) {
            result = buffer_pool->acquire(buffer_size);
        } else {
            result.resize(buffer_size);
        }

        // Параллельная обработка строк изображения
        ParallelImageProcessor::processRowsParallel(
            height,
            [input_data, &result, &kernel, &border_handler, width, channels, kernel_size, kernel_radius](
        int start_row, int end_row) {
                // Обрабатываем строки в диапазоне [start_row, end_row)
                for (int y = start_row; y < end_row; ++y) {
                    const auto row_offset = static_cast<size_t>(y) * static_cast<size_t>(width) * static_cast<size_t>(
                                                channels);

                    for (int x = 0; x < width; ++x) {
                        // Обрабатываем каждый канал отдельно
                        for (int c = 0; c < channels; ++c) {
                            int64_t sum = 0;

                            // Применяем ядро по горизонтали
                            for (int k = 0; k < kernel_size; ++k) {
                                const auto sample_x = x + k - kernel_radius;

                                // Обработка границ с использованием BorderHandler
                                const auto clamped_x = border_handler.getX(sample_x, width);

                                const auto pixel_index =
                                        row_offset + static_cast<size_t>(clamped_x) * static_cast<size_t>(channels) +
                                        static_cast<size_t>(c);
                                sum += static_cast<int64_t>(input_data[pixel_index]) * kernel[static_cast<size_t>(k)];
                            }

                            // Деление на масштаб с округлением
                            // KERNEL_SCALE всегда > 0, но добавляем проверку для безопасности
                            if (KERNEL_SCALE > 0) {
                                // Защита от переполнения: проверяем, что sum не слишком большой
                                const int64_t max_safe_sum = static_cast<int64_t>(INT_MAX) * KERNEL_SCALE;
                                if (sum > max_safe_sum) {
                                    sum = max_safe_sum;
                                } else if (sum < -max_safe_sum) {
                                    sum = -max_safe_sum;
                                }

                                const auto result_value = static_cast<int>((sum + (KERNEL_SCALE / 2)) / KERNEL_SCALE);
                                const auto result_index =
                                        row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels) +
                                        static_cast<size_t>(c);
                                result[result_index] = static_cast<uint8_t>(std::max(0, std::min(255, result_value)));
                            } else {
                                // Fallback: используем исходное значение
                                const auto result_index =
                                        row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels) +
                                        static_cast<size_t>(c);
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
     * @param buffer_pool Пул буферов для переиспользования (может быть nullptr)
     * @return Финальный результат размытия
     */
    std::vector<uint8_t> applyVerticalKernel(
        const std::vector<uint8_t> &horizontalResult,
        const ImageProcessor &image,
        const std::vector<int32_t> &kernel,
        const BorderHandler &border_handler,
        IBufferPool *buffer_pool
    ) {
        const auto width = image.getWidth();
        const auto height = image.getHeight();
        const auto channels = image.getChannels();
        const auto kernel_size = static_cast<int>(kernel.size());
        const auto kernel_radius = kernel_size / 2;

        size_t width_height_product = 0;
        size_t buffer_size = 0;
        if (!SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), width_height_product) ||
            !SafeMath::safeMultiply(width_height_product, static_cast<size_t>(channels), buffer_size)) {
            // Возвращаем пустой вектор при переполнении
            return std::vector<uint8_t>();
        }

        // Получаем буфер из пула или создаем новый
        std::vector<uint8_t> result;
        if (buffer_pool != nullptr) {
            result = buffer_pool->acquire(buffer_size);
        } else {
            result.resize(buffer_size);
        }

        // Параллельная обработка строк изображения
        ParallelImageProcessor::processRowsParallel(
            height,
            [&horizontalResult, &result, &kernel, &border_handler, width, height, channels, kernel_size, kernel_radius](
        int start_row, int end_row) {
                // Обрабатываем строки в диапазоне [start_row, end_row)
                for (int y = start_row; y < end_row; ++y) {
                    const auto row_offset = static_cast<size_t>(y) * static_cast<size_t>(width) * static_cast<size_t>(
                                                channels);

                    for (int x = 0; x < width; ++x) {
                        // Обрабатываем каждый канал отдельно
                        for (int c = 0; c < channels; ++c) {
                            int64_t sum = 0;

                            // Применяем ядро по вертикали
                            for (int k = 0; k < kernel_size; ++k) {
                                const auto sample_y = y + k - kernel_radius;

                                // Обработка границ с использованием BorderHandler
                                const auto clamped_y = border_handler.getY(sample_y, height);

                                const auto pixel_index =
                                (static_cast<size_t>(clamped_y) * static_cast<size_t>(width) + static_cast<
                                     size_t>(x)) * static_cast<size_t>(channels) + static_cast<size_t>(c);
                                sum += static_cast<int64_t>(horizontalResult[pixel_index]) * kernel[static_cast<size_t>(
                                    k)];
                            }

                            // Деление на масштаб с округлением
                            // KERNEL_SCALE всегда > 0, но добавляем проверку для безопасности
                            if (KERNEL_SCALE > 0) {
                                // Защита от переполнения: проверяем, что sum не слишком большой
                                const int64_t max_safe_sum = static_cast<int64_t>(INT_MAX) * KERNEL_SCALE;
                                if (sum > max_safe_sum) {
                                    sum = max_safe_sum;
                                } else if (sum < -max_safe_sum) {
                                    sum = -max_safe_sum;
                                }

                                const auto result_value = static_cast<int>((sum + (KERNEL_SCALE / 2)) / KERNEL_SCALE);
                                const auto result_index =
                                        row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels) +
                                        static_cast<size_t>(c);
                                result[result_index] = static_cast<uint8_t>(std::max(0, std::min(255, result_value)));
                            } else {
                                // Fallback: используем исходное значение
                                const auto result_index =
                                        row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels) +
                                        static_cast<size_t>(c);
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

FilterResult GaussianBlurFilter::apply(ImageProcessor &image) {
    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();

    // Валидация параметра фильтра (радиус должен быть > 0 для Gaussian blur)
    auto radius_result = FilterValidator::validateRadius(radius_, 0.001, 1000.0, width, height);

    // Валидация изображения и параметра с автоматическим добавлением контекста
    auto validation_result = FilterValidationHelper::validateImageAndParam(
        image, radius_result, "radius", radius_);
    if (validation_result.hasError()) {
        return validation_result;
    }

    // Вычисляем стандартное отклонение (sigma) из радиуса
    // Эмпирическое правило: sigma ≈ radius / 2
    auto sigma = radius_ / 2.0;

    // Получаем ядро из кэша или генерируем новое
    auto kernel = getOrGenerateKernel(radius_, sigma);

    // Применяем separable kernel: сначала по горизонтали, затем по вертикали
    // Это оптимизация: вместо O(N²) операций на пиксель получаем O(2N)

    auto horizontal_result = applyHorizontalKernel(image, kernel, border_handler_, buffer_pool_);
    auto final_result = applyVerticalKernel(horizontal_result, image, kernel, border_handler_, buffer_pool_);

    // Копируем результат обратно в изображение
    auto *data = image.getData();
    std::ranges::copy(final_result, data);

    // Возвращаем буферы в пул для переиспользования
    if (buffer_pool_ != nullptr) {
        buffer_pool_->release(std::move(horizontal_result));
        buffer_pool_->release(std::move(final_result));
    }

    return FilterResult::success();
}


std::string GaussianBlurFilter::getName() const {
    return "blur";
}

std::string GaussianBlurFilter::getDescription() const {
    return "Размытие по Гауссу";
}

std::string GaussianBlurFilter::getCategory() const {
    return "Размытие и шум";
}
