#include <filters/EdgeDetectionFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/FilterValidator.h>
#include <utils/FilterValidationHelper.h>
#include <utils/BorderHandler.h>
#include <utils/LookupTables.h>
#include <utils/ColorConversionUtils.h>
#include <cmath>
#include <vector>
#include <array>

namespace {
    /**
     * @brief Возвращает ядра оператора Собеля
     * @param gx Ядро для горизонтального градиента (выходной параметр)
     * @param gy Ядро для вертикального градиента (выходной параметр)
     */
    void getSobelKernels(std::array<std::array<int, 3>, 3>& gx, std::array<std::array<int, 3>, 3>& gy)
    {
        // Gx (горизонтальное)
        gx[0] = {{-1, 0, 1}};
        gx[1] = {{-2, 0, 2}};
        gx[2] = {{-1, 0, 1}};

        // Gy (вертикальное)
        gy[0] = {{-1, -2, -1}};
        gy[1] = {{0, 0, 0}};
        gy[2] = {{1, 2, 1}};
    }

    /**
     * @brief Возвращает ядра оператора Преввитта
     * @param gx Ядро для горизонтального градиента (выходной параметр)
     * @param gy Ядро для вертикального градиента (выходной параметр)
     */
    void getPrewittKernels(std::array<std::array<int, 3>, 3>& gx, std::array<std::array<int, 3>, 3>& gy)
    {
        // Gx (горизонтальное)
        gx[0] = {{-1, 0, 1}};
        gx[1] = {{-1, 0, 1}};
        gx[2] = {{-1, 0, 1}};

        // Gy (вертикальное)
        gy[0] = {{-1, -1, -1}};
        gy[1] = {{0, 0, 0}};
        gy[2] = {{1, 1, 1}};
    }

    /**
     * @brief Возвращает ядра оператора Шарра
     * @param gx Ядро для горизонтального градиента (выходной параметр)
     * @param gy Ядро для вертикального градиента (выходной параметр)
     */
    void getScharrKernels(std::array<std::array<int, 3>, 3>& gx, std::array<std::array<int, 3>, 3>& gy)
    {
        // Gx (горизонтальное) - оптимизированная версия для лучшей точности
        gx[0] = {{-3, 0, 3}};
        gx[1] = {{-10, 0, 10}};
        gx[2] = {{-3, 0, 3}};

        // Gy (вертикальное)
        gy[0] = {{-3, -10, -3}};
        gy[1] = {{0, 0, 0}};
        gy[2] = {{3, 10, 3}};
    }
}

FilterResult EdgeDetectionFilter::apply(ImageProcessor& image)
{
    // Валидация параметра фильтра (sensitivity обычно валидируется отдельно, если нужно)
    // Для EdgeDetectionFilter sensitivity обычно не требует валидации, но добавим для консистентности
    auto validation_result = FilterValidationHelper::validateImageWithParam(
        image, "sensitivity", sensitivity_);
    if (validation_result.hasError())
    {
        return validation_result;
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();

    // Валидация параметра фильтра
    auto sensitivity_result = FilterValidator::validateIntensity(sensitivity_);
    if (sensitivity_result.hasError())
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        ctx.withFilterParam("sensitivity", sensitivity_);
        return FilterResult::failure(sensitivity_result.error, sensitivity_result.message, ctx);
    }

    // Инициализируем lookup tables
    LookupTables::initialize();

    const auto* input_data = image.getData();

    // Сначала преобразуем в градации серого
    const auto grayscale_size = static_cast<size_t>(width) * static_cast<size_t>(height);
    
    // Создаем буфер для градаций серого
    std::vector<uint8_t> grayscale(grayscale_size);

    // Коэффициенты для преобразования RGB в градации серого

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            const auto pixel_offset = (static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * static_cast<size_t>(channels);
            const auto r = static_cast<int>(input_data[pixel_offset + 0]);
            const auto g = static_cast<int>(input_data[pixel_offset + 1]);
            const auto b = static_cast<int>(input_data[pixel_offset + 2]);
            // Используем общую утилиту для преобразования RGB в градации серого
            grayscale[static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)] = 
                ColorConversionUtils::rgbToGrayscale(r, g, b);
        }
    }


    std::vector<int> gradient_magnitude(static_cast<size_t>(width) * static_cast<size_t>(height));

    // Получаем ядра в зависимости от выбранного оператора
    std::array<std::array<int, 3>, 3> gx_kernel, gy_kernel;
    switch (operator_type_)
    {
        case EdgeDetectionFilter::Operator::Sobel:
            getSobelKernels(gx_kernel, gy_kernel);
            break;
        case EdgeDetectionFilter::Operator::Prewitt:
            getPrewittKernels(gx_kernel, gy_kernel);
            break;
        case EdgeDetectionFilter::Operator::Scharr:
            getScharrKernels(gx_kernel, gy_kernel);
            break;
    }

    // Применяем выбранный оператор
    ParallelImageProcessor::processRowsParallel(
        height,
        [width, height, &grayscale, &gradient_magnitude, &gx_kernel, &gy_kernel, this](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    int gx = 0;
                    int gy = 0;

                    // Применяем ядра оператора
                    for (int ky = -1; ky <= 1; ++ky)
                    {
                        for (int kx = -1; kx <= 1; ++kx)
                        {
                            const auto px = x + kx;
                            const auto py = y + ky;

                            // Обработка границ с использованием BorderHandler
                            const auto clamped_x = border_handler_.getX(px, width);
                            const auto clamped_y = border_handler_.getY(py, height);

                            const auto pixel_value = static_cast<int>(grayscale[static_cast<size_t>(clamped_y) * static_cast<size_t>(width) +
                                static_cast<size_t>(clamped_x)]);
                            gx += pixel_value * gx_kernel[static_cast<size_t>(ky + 1)][static_cast<size_t>(kx + 1)];
                            gy += pixel_value * gy_kernel[static_cast<size_t>(ky + 1)][static_cast<size_t>(kx + 1)];
                        }
                    }

                    // Вычисляем магнитуду градиента
                    // Используем lookup table для sqrt для оптимизации
                    const auto gradient_squared = gx * gx + gy * gy;
                    gradient_magnitude[static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)] = 
                        static_cast<int>(LookupTables::sqrtInt(gradient_squared));
                }
            }
        }
    );

    // Нормализуем и применяем к изображению с учетом чувствительности
    int max_gradient = 0;
    for (int val : gradient_magnitude)
    {
        if (val > max_gradient)
        {
            max_gradient = val;
        }
    }

    // Применяем чувствительность: чем выше чувствительность, тем ниже порог нормализации
    // Это позволяет выделять более слабые края при высокой чувствительности
    const auto threshold = static_cast<int>(max_gradient * (1.0 - sensitivity_));
    const auto effective_max = max_gradient - threshold;

    auto* data = image.getData();
    if (effective_max > 0)
    {
        ParallelImageProcessor::processRowsParallel(
            height,
            [width, channels, &gradient_magnitude, data, threshold, effective_max](int start_row, int end_row)
            {
                for (int y = start_row; y < end_row; ++y)
                {
                    for (int x = 0; x < width; ++x)
                    {
                        const auto pixel_offset = (static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * static_cast<size_t>(channels);
                        auto gradient = gradient_magnitude[static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)];
                        
                        // Применяем порог с учетом чувствительности
                        gradient = std::max(0, gradient - threshold);
                        
                        // Нормализуем в диапазон [0, 255]
                        const auto normalized = static_cast<uint8_t>((gradient * 255) / effective_max);

                        data[pixel_offset + 0] = normalized;
                        data[pixel_offset + 1] = normalized;
                        data[pixel_offset + 2] = normalized;
                    }
                }
            }
        );
    }
    else
    {
        // Если нет градиентов выше порога, заполняем черным
        ParallelImageProcessor::processRowsParallel(
            height,
            [width, channels, data](int start_row, int end_row)
            {
                for (int y = start_row; y < end_row; ++y)
                {
                    for (int x = 0; x < width; ++x)
                    {
                        const auto pixel_offset = (static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * static_cast<size_t>(channels);
                        data[pixel_offset + 0] = 0;
                        data[pixel_offset + 1] = 0;
                        data[pixel_offset + 2] = 0;
                    }
                }
            }
        );
    }

    return FilterResult::success();
}

std::string EdgeDetectionFilter::getName() const
{
    return "edges";
}

std::string EdgeDetectionFilter::getDescription() const
{
    std::string op_name;
    switch (operator_type_)
    {
        case Operator::Sobel:
            op_name = "Собеля";
            break;
        case Operator::Prewitt:
            op_name = "Преввитта";
            break;
        case Operator::Scharr:
            op_name = "Шарра";
            break;
    }
    return "Детекция краёв (оператор " + op_name + ")";
}

std::string EdgeDetectionFilter::getCategory() const
{
    return "Края и детали";
}



