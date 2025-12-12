#include <filters/Rotate90Filter.h>
#include <ImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/FilterValidationHelper.h>
#include <utils/IBufferPool.h>
#include <utils/SafeMath.h>
#include <cerrno>
#include <cstring>
#include <vector>

FilterResult Rotate90Filter::apply(ImageProcessor& image)
{
    // Базовая валидация изображения
    auto validation_result = FilterValidationHelper::validateImageWithParam(
        image, "clockwise", std::string(clockwise_ ? "true" : "false"));
    if (validation_result.hasError())
    {
        return validation_result;
    }

    const auto old_width = image.getWidth();
    const auto old_height = image.getHeight();
    const auto channels = image.getChannels();

    const auto* old_data = image.getData();

    // Новые размеры после поворота
    const auto new_width = old_height;
    const auto new_height = old_width;

    // Создаем новый буфер для повернутого изображения
    size_t width_height_product = 0;
    size_t buffer_size = 0;
    if (!SafeMath::safeMultiply(static_cast<size_t>(new_width), static_cast<size_t>(new_height), width_height_product) ||
        !SafeMath::safeMultiply(width_height_product, static_cast<size_t>(channels), buffer_size))
    {
        ErrorContext ctx = ErrorContext::withImage(old_width, old_height, channels);
        ctx.withFilterParam("clockwise", std::string(clockwise_ ? "true" : "false"));
        return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                   "Размер изображения слишком большой", ctx);
    }
    
    // Получаем временный буфер из пула или создаем новый
    std::vector<uint8_t> temp_buffer;
    if (buffer_pool_ != nullptr)
    {
        temp_buffer = buffer_pool_->acquire(buffer_size);
    }
    else
    {
        temp_buffer.resize(buffer_size);
    }
    
    // Получаем указатель на данные временного буфера
    uint8_t* new_data = temp_buffer.data();

    // Предвычисляем размеры для оптимизации
    const auto old_row_stride = static_cast<size_t>(old_width) * static_cast<size_t>(channels);
    const auto new_row_stride = static_cast<size_t>(new_width) * static_cast<size_t>(channels);
    const auto channels_size = static_cast<size_t>(channels);

    if (clockwise_)
    {
        // Поворот по часовой стрелке на 90 градусов
        // Пиксель (x, y) в исходном изображении переходит в (old_height - 1 - y, x) в новом
        // Оптимизация: переставляем циклы для лучшей локальности данных
        for (int x = 0; x < old_width; ++x)
        {
            for (int y = 0; y < old_height; ++y)
            {
                const auto old_offset = static_cast<size_t>(y) * old_row_stride + static_cast<size_t>(x) * channels_size;
                
                // Новые координаты после поворота по часовой стрелке
                const auto new_x = old_height - 1 - y;
                const auto new_y = x;
                const auto new_offset = static_cast<size_t>(new_y) * new_row_stride + static_cast<size_t>(new_x) * channels_size;

                // Копируем все каналы сразу через memcpy для лучшей производительности
                std::memcpy(new_data + new_offset, old_data + old_offset, channels_size);
            }
        }
    }
    else
    {
        // Поворот против часовой стрелки на 90 градусов
        // Пиксель (x, y) в исходном изображении переходит в (y, old_width - 1 - x) в новом
        // Оптимизация: переставляем циклы для лучшей локальности данных
        for (int x = 0; x < old_width; ++x)
        {
            for (int y = 0; y < old_height; ++y)
            {
                const auto old_offset = static_cast<size_t>(y) * old_row_stride + static_cast<size_t>(x) * channels_size;
                
                // Новые координаты после поворота против часовой стрелки
                const auto new_x = y;
                const auto new_y = old_width - 1 - x;
                const auto new_offset = static_cast<size_t>(new_y) * new_row_stride + static_cast<size_t>(new_x) * channels_size;

                // Копируем все каналы сразу через memcpy для лучшей производительности
                std::memcpy(new_data + new_offset, old_data + old_offset, channels_size);
            }
        }
    }
    
    // Передаем данные напрямую в resize() - он сам скопирует их в malloc-выделенную память
    // После этого мы можем вернуть временный буфер в пул
    const auto resize_result = image.resize(new_width, new_height, new_data);
    
    // Возвращаем временный буфер в пул (после того как resize() скопировал данные)
    if (buffer_pool_ != nullptr)
    {
        buffer_pool_->release(std::move(temp_buffer));
    }
    if (!resize_result.isSuccess())
    {
        return resize_result;
    }
    
    return FilterResult::success();
}

std::string Rotate90Filter::getName() const
{
    return "rotate90";
}

std::string Rotate90Filter::getDescription() const
{
    return "Поворот на 90 градусов";
}

std::string Rotate90Filter::getCategory() const
{
    return "Геометрический";
}

