#include <filters/Rotate90Filter.h>
#include <ImageProcessor.h>
#include <utils/FilterResult.h>
#include <cerrno>
#include <vector>
#include <cstring>

FilterResult Rotate90Filter::apply(ImageProcessor& image)
{
    if (!image.isValid())
    {
        return FilterResult::failure(FilterError::InvalidImage, "Изображение не загружено");
    }

    const auto old_width = image.getWidth();
    const auto old_height = image.getHeight();
    const auto channels = image.getChannels();

    // Валидация размеров изображения
    if (old_width <= 0 || old_height <= 0)
    {
        ErrorContext ctx = ErrorContext::withImage(old_width, old_height, channels);
        ctx.filter_params = "clockwise=" + std::string(clockwise_ ? "true" : "false");
        return FilterResult::failure(FilterError::InvalidSize,
                                     "Размер изображения должен быть больше нуля", ctx);
    }

    if (channels != 3 && channels != 4)
    {
        ErrorContext ctx = ErrorContext::withImage(old_width, old_height, channels);
        ctx.filter_params = "clockwise=" + std::string(clockwise_ ? "true" : "false");
        return FilterResult::failure(FilterError::InvalidChannels, 
                                     "Ожидается 3 канала (RGB) или 4 канала (RGBA), получено: " + std::to_string(channels),
                                     ctx);
    }

    const auto* old_data = image.getData();

    // Новые размеры после поворота
    const auto new_width = old_height;
    const auto new_height = old_width;

    // Создаем новый буфер для повернутого изображения
    const auto buffer_size = static_cast<size_t>(new_width) * static_cast<size_t>(new_height) * static_cast<size_t>(channels);
    
    // Создаем буфер для повернутого изображения
    std::vector<uint8_t> new_data(buffer_size);

    if (clockwise_)
    {
        // Поворот по часовой стрелке на 90 градусов
        // Пиксель (x, y) в исходном изображении переходит в (y, new_width - 1 - x) в новом
        for (int y = 0; y < old_height; ++y)
        {
            for (int x = 0; x < old_width; ++x)
            {
                const auto old_offset = (static_cast<size_t>(y) * static_cast<size_t>(old_width) + static_cast<size_t>(x)) * static_cast<size_t>(channels);
                
                // Новые координаты после поворота по часовой стрелке
                const auto new_x = old_height - 1 - y;
                const auto new_y = x;
                const auto new_offset = (static_cast<size_t>(new_y) * static_cast<size_t>(new_width) + static_cast<size_t>(new_x)) * static_cast<size_t>(channels);

                for (int c = 0; c < channels; ++c)
                {
                    new_data[new_offset + static_cast<size_t>(c)] = old_data[old_offset + static_cast<size_t>(c)];
                }
            }
        }
    }
    else
    {
        // Поворот против часовой стрелки на 90 градусов
        // Пиксель (x, y) в исходном изображении переходит в (new_height - 1 - y, x) в новом
        for (int y = 0; y < old_height; ++y)
        {
            for (int x = 0; x < old_width; ++x)
            {
                const auto old_offset = (static_cast<size_t>(y) * static_cast<size_t>(old_width) + static_cast<size_t>(x)) * static_cast<size_t>(channels);
                
                // Новые координаты после поворота против часовой стрелки
                const auto new_x = y;
                const auto new_y = old_width - 1 - x;
                const auto new_offset = (static_cast<size_t>(new_y) * static_cast<size_t>(new_width) + static_cast<size_t>(new_x)) * static_cast<size_t>(channels);

                for (int c = 0; c < channels; ++c)
                {
                    new_data[new_offset + static_cast<size_t>(c)] = old_data[old_offset + static_cast<size_t>(c)];
                }
            }
        }
    }

    // Используем метод resize для изменения размеров и замены данных
    // Создаем копию данных, так как resize принимает владение над данными
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(buffer_size));
    if (allocated_data == nullptr)
    {
        const int errno_code = errno;
        ErrorContext ctx = ErrorContext::withImage(old_width, old_height, channels);
        ctx.filter_params = "clockwise=" + std::string(clockwise_ ? "true" : "false");
        if (errno_code != 0)
        {
            ctx.system_error_code = errno_code;
        }
        return FilterResult::failure(FilterError::MemoryAllocationFailed, 
                                     "Не удалось выделить память для изображения", ctx);
    }
    std::memcpy(allocated_data, new_data.data(), buffer_size);
    
    if (!image.resize(new_width, new_height, allocated_data))
    {
        return FilterResult::failure(FilterError::OutOfMemory, "Не удалось изменить размер изображения");
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

