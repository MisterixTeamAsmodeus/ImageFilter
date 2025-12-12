#include <filters/Rotate90Filter.h>
#include <ImageProcessor.h>
#include <vector>
#include <cstring>

bool Rotate90Filter::apply(ImageProcessor& image)
{
    if (!image.isValid())
    {
        return false;
    }

    const auto old_width = image.getWidth();
    const auto old_height = image.getHeight();
    const auto channels = image.getChannels();

    if (channels != 3)
    {
        return false;
    }

    const auto* old_data = image.getData();

    // Новые размеры после поворота
    const auto new_width = old_height;
    const auto new_height = old_width;

    // Создаем новый буфер для повернутого изображения
    std::vector<uint8_t> new_data(static_cast<size_t>(new_width) * new_height * channels);

    if (clockwise_)
    {
        // Поворот по часовой стрелке на 90 градусов
        // Пиксель (x, y) в исходном изображении переходит в (y, new_width - 1 - x) в новом
        for (int y = 0; y < old_height; ++y)
        {
            for (int x = 0; x < old_width; ++x)
            {
                const auto old_offset = (static_cast<size_t>(y) * old_width + x) * channels;
                
                // Новые координаты после поворота по часовой стрелке
                const auto new_x = old_height - 1 - y;
                const auto new_y = x;
                const auto new_offset = (static_cast<size_t>(new_y) * new_width + new_x) * channels;

                for (int c = 0; c < channels; ++c)
                {
                    new_data[new_offset + c] = old_data[old_offset + c];
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
                const auto old_offset = (static_cast<size_t>(y) * old_width + x) * channels;
                
                // Новые координаты после поворота против часовой стрелки
                const auto new_x = y;
                const auto new_y = old_width - 1 - x;
                const auto new_offset = (static_cast<size_t>(new_y) * new_width + new_x) * channels;

                for (int c = 0; c < channels; ++c)
                {
                    new_data[new_offset + c] = old_data[old_offset + c];
                }
            }
        }
    }

    // Используем метод resize для изменения размеров и замены данных
    return image.resize(new_width, new_height, new_data.data());
}
