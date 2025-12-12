#include <filters/GaussianBlurFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <algorithm>
#include <vector>
#include <numbers>  // C++20: для std::numbers::pi

// Масштаб для целочисленной арифметики (16 бит для точности)
constexpr int32_t KERNEL_SCALE = 65536;

bool GaussianBlurFilter::apply(ImageProcessor& image)
{
    if (!image.isValid())
    {
        return false;
    }

    if (image.getChannels() != 3)
    {
        return false;
    }

    // Вычисляем стандартное отклонение (sigma) из радиуса
    // Эмпирическое правило: sigma ≈ radius / 2
    auto sigma = radius_ / 2.0;

    // Генерируем одномерное ядро Гаусса в целочисленном формате
    auto kernel = generateKernel(radius_, sigma);
    normalizeKernel(kernel);

    // Применяем separable kernel: сначала по горизонтали, затем по вертикали
    // Это оптимизация: вместо O(N²) операций на пиксель получаем O(2N)
    auto horizontal_result = applyHorizontalKernel(image, kernel);
    auto final_result = applyVerticalKernel(horizontal_result, image, kernel);

    // Копируем результат обратно в изображение
    auto* data = image.getData();
    std::ranges::copy(final_result, data);

    return true;
}

std::vector<int32_t> GaussianBlurFilter::generateKernel(const double radius, const double& sigma)
{
    // Размер ядра должен быть нечетным и покрывать 3 сигмы в каждую сторону
    // Это обеспечивает, что 99.7% веса функции Гаусса попадет в ядро
    const auto kernel_size = static_cast<int>(std::ceil(radius * 2.0)) | 1; // Делаем нечетным
    const auto center = kernel_size / 2;

    auto kernel = std::vector<int32_t>(kernel_size);
    const auto two_sigma_squared = 2.0 * sigma * sigma;
    // Используем std::numbers::pi из C++20 вместо M_PI для переносимости
    const auto sqrt_two_pi_sigma = std::sqrt(2.0 * std::numbers::pi) * sigma;

    // Генерируем значения функции Гаусса для каждого элемента ядра
    // Масштабируем на KERNEL_SCALE для целочисленной арифметики
    for (int i = 0; i < kernel_size; ++i)
    {
        const auto distance = static_cast<double>(i - center);
        // Формула функции Гаусса: G(x) = (1 / (σ * √(2π))) * e^(-(x² / (2σ²))))
        const auto value = std::exp(-(distance * distance) / two_sigma_squared) / sqrt_two_pi_sigma;
        kernel[i] = static_cast<int32_t>(value * KERNEL_SCALE);
    }

    return kernel;
}

void GaussianBlurFilter::normalizeKernel(std::vector<int32_t>& kernel)
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

std::vector<uint8_t> GaussianBlurFilter::applyHorizontalKernel(
    const ImageProcessor& image,
    const std::vector<int32_t>& kernel
)
{
    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();
    const auto kernel_size = static_cast<int>(kernel.size());
    const auto kernel_radius = kernel_size / 2;

    const auto* input_data = image.getData();
    auto result = std::vector<uint8_t>(static_cast<size_t>(width) * height * channels);

    // Параллельная обработка строк изображения
    ParallelImageProcessor::processRowsParallel(
        height,
        [width, channels, kernel_size, kernel_radius, input_data, &result, &kernel](int start_row, int end_row)
        {
            // Обрабатываем строки в диапазоне [start_row, end_row)
            for (int y = start_row; y < end_row; ++y)
            {
                const auto row_offset = static_cast<size_t>(y) * width * channels;

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

                            // Обработка границ: используем отражение (mirroring)
                            // Это предотвращает артефакты на краях изображения
                            int clamped_x = sample_x;
                            if (clamped_x < 0)
                            {
                                clamped_x = -clamped_x; // Отражение
                            }
                            else if (clamped_x >= width)
                            {
                                clamped_x = 2 * width - clamped_x - 1; // Отражение
                            }

                            const auto pixel_index = row_offset + static_cast<size_t>(clamped_x) * channels + c;
                            sum += static_cast<int64_t>(input_data[pixel_index]) * kernel[k];
                        }

                        // Деление на масштаб с округлением
                        const auto result_value = static_cast<int>((sum + (KERNEL_SCALE / 2)) / KERNEL_SCALE);
                        const auto result_index = row_offset + static_cast<size_t>(x) * channels + c;
                        result[result_index] = static_cast<uint8_t>(std::max(0, std::min(255, result_value)));
                    }
                }
            }
        }
    );

    return result;
}

std::vector<uint8_t> GaussianBlurFilter::applyVerticalKernel(
    const std::vector<uint8_t>& horizontalResult,
    const ImageProcessor& image,
    const std::vector<int32_t>& kernel
)
{
    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();
    const auto kernel_size = static_cast<int>(kernel.size());
    const auto kernel_radius = kernel_size / 2;

    std::vector<uint8_t> result(static_cast<size_t>(width) * height * channels);

    // Параллельная обработка строк изображения
    ParallelImageProcessor::processRowsParallel(
        height,
        [width, height, channels, kernel_size, kernel_radius, &horizontalResult, &result, &kernel](
        int start_row, int end_row)
        {
            // Обрабатываем строки в диапазоне [start_row, end_row)
            for (int y = start_row; y < end_row; ++y)
            {
                const auto row_offset = static_cast<size_t>(y) * width * channels;

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

                            // Обработка границ: используем отражение
                            int clamped_y = sample_y;
                            if (clamped_y < 0)
                            {
                                clamped_y = -clamped_y;
                            }
                            else if (clamped_y >= height)
                            {
                                clamped_y = 2 * height - clamped_y - 1;
                            }

                            const auto pixel_index = (static_cast<size_t>(clamped_y) * width + x) * channels + c;
                            sum += static_cast<int64_t>(horizontalResult[pixel_index]) * kernel[k];
                        }

                        // Деление на масштаб с округлением
                        const auto result_value = static_cast<int>((sum + (KERNEL_SCALE / 2)) / KERNEL_SCALE);
                        const auto result_index = row_offset + static_cast<size_t>(x) * channels + c;
                        result[result_index] = static_cast<uint8_t>(std::max(0, std::min(255, result_value)));
                    }
                }
            }
        }
    );

    return result;
}
