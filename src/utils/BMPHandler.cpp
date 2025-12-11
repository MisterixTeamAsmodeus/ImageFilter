#include <utils/BMPHandler.h>
#include <fstream>
#include <algorithm>
#include <cstring>

namespace BMPHandler
{
    /**
     * @brief Структура заголовка BMP файла
     */
    #pragma pack(push, 1)
    struct BMPHeader
    {
        uint16_t signature;      // "BM" (0x4D42)
        uint32_t file_size;      // Размер файла в байтах
        uint16_t reserved1;      // Зарезервировано
        uint16_t reserved2;      // Зарезервировано
        uint32_t data_offset;     // Смещение до начала данных изображения
    };

    struct BMPInfoHeader
    {
        uint32_t header_size;    // Размер этого заголовка (40 байт)
        int32_t width;           // Ширина изображения в пикселях
        int32_t height;          // Высота изображения в пикселях (может быть отрицательным)
        uint16_t planes;         // Количество плоскостей (всегда 1)
        uint16_t bits_per_pixel; // Бит на пиксель (24 для RGB)
        uint32_t compression;    // Тип сжатия (0 = без сжатия)
        uint32_t image_size;     // Размер данных изображения в байтах
        int32_t x_resolution;    // Горизонтальное разрешение (пикселей на метр)
        int32_t y_resolution;    // Вертикальное разрешение (пикселей на метр)
        uint32_t colors_used;    // Количество используемых цветов (0 = все)
        uint32_t important_colors; // Количество важных цветов (0 = все)
    };
    #pragma pack(pop)

    uint8_t* loadBMP(const std::string& filename, int& width, int& height, int& channels)
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open())
        {
            return nullptr;
        }

        // Читаем заголовок
        BMPHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(header));
        
        if (!file.good() || header.signature != 0x4D42) // "BM"
        {
            return nullptr;
        }

        // Читаем информационный заголовок
        BMPInfoHeader info_header;
        file.read(reinterpret_cast<char*>(&info_header), sizeof(info_header));
        
        if (!file.good() || info_header.header_size < 40)
        {
            return nullptr;
        }

        // Проверяем, что это 24-битное RGB изображение без сжатия
        if (info_header.bits_per_pixel != 24 || info_header.compression != 0)
        {
            return nullptr;
        }

        width = info_header.width;
        height = std::abs(info_header.height); // Высота может быть отрицательной (верх-вниз)
        channels = 3; // BMP всегда RGB

        if (width <= 0 || height <= 0)
        {
            return nullptr;
        }

        // Вычисляем размер строки с учетом выравнивания (должно быть кратно 4 байтам)
        const int row_size = ((width * 3 + 3) / 4) * 4;

        // Перемещаемся к началу данных изображения
        file.seekg(header.data_offset, std::ios::beg);
        if (!file.good())
        {
            return nullptr;
        }

        // Выделяем память для данных
        uint8_t* image_data = static_cast<uint8_t*>(std::malloc(static_cast<size_t>(width) * static_cast<size_t>(height) * 3));
        if (image_data == nullptr)
        {
            return nullptr;
        }

        // Читаем данные построчно (BMP хранит данные снизу вверх)
        const bool top_down = info_header.height < 0;
        
        if (top_down)
        {
            // Изображение хранится сверху вниз
            for (int y = 0; y < height; ++y)
            {
                std::vector<uint8_t> row(static_cast<size_t>(row_size));
                file.read(reinterpret_cast<char*>(row.data()), static_cast<std::streamsize>(row_size));
                
                if (!file.good())
                {
                    std::free(image_data);
                    return nullptr;
                }

                // Копируем RGB данные (BMP хранит в формате BGR)
                for (int x = 0; x < width; ++x)
                {
                    const size_t src_offset = static_cast<size_t>(x) * 3;
                    const size_t dst_offset = (static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * 3;
                    
                    // Конвертируем BGR в RGB
                    image_data[dst_offset + 0] = row[src_offset + 2]; // R
                    image_data[dst_offset + 1] = row[src_offset + 1]; // G
                    image_data[dst_offset + 2] = row[src_offset + 0]; // B
                }
            }
        }
        else
        {
            // Изображение хранится снизу вверх
            for (int y = height - 1; y >= 0; --y)
            {
                std::vector<uint8_t> row(static_cast<size_t>(row_size));
                file.read(reinterpret_cast<char*>(row.data()), static_cast<std::streamsize>(row_size));
                
                if (!file.good())
                {
                    std::free(image_data);
                    return nullptr;
                }

                // Копируем RGB данные (BMP хранит в формате BGR)
                for (int x = 0; x < width; ++x)
                {
                    const size_t src_offset = static_cast<size_t>(x) * 3;
                    const size_t dst_offset = (static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * 3;
                    
                    // Конвертируем BGR в RGB
                    image_data[dst_offset + 0] = row[src_offset + 2]; // R
                    image_data[dst_offset + 1] = row[src_offset + 1]; // G
                    image_data[dst_offset + 2] = row[src_offset + 0]; // B
                }
            }
        }

        return image_data;
    }

    bool saveBMP(const std::string& filename, int width, int height, int channels, const uint8_t* data)
    {
        if (width <= 0 || height <= 0 || data == nullptr)
        {
            return false;
        }

        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open())
        {
            return false;
        }

        // Вычисляем размер строки с учетом выравнивания (должно быть кратно 4 байтам)
        const int row_size = ((width * 3 + 3) / 4) * 4;
        const size_t image_data_size = static_cast<size_t>(row_size) * static_cast<size_t>(height);
        const uint32_t data_offset = sizeof(BMPHeader) + sizeof(BMPInfoHeader);
        const uint32_t file_size = data_offset + static_cast<uint32_t>(image_data_size);

        // Записываем заголовок
        BMPHeader header;
        header.signature = 0x4D42; // "BM"
        header.file_size = file_size;
        header.reserved1 = 0;
        header.reserved2 = 0;
        header.data_offset = data_offset;
        
        file.write(reinterpret_cast<const char*>(&header), sizeof(header));
        if (!file.good())
        {
            return false;
        }

        // Записываем информационный заголовок
        BMPInfoHeader info_header;
        info_header.header_size = 40;
        info_header.width = width;
        info_header.height = height; // Положительное значение = снизу вверх
        info_header.planes = 1;
        info_header.bits_per_pixel = 24;
        info_header.compression = 0; // Без сжатия
        info_header.image_size = static_cast<uint32_t>(image_data_size);
        info_header.x_resolution = 0;
        info_header.y_resolution = 0;
        info_header.colors_used = 0;
        info_header.important_colors = 0;
        
        file.write(reinterpret_cast<const char*>(&info_header), sizeof(info_header));
        if (!file.good())
        {
            return false;
        }

        // Записываем данные изображения построчно (BMP хранит снизу вверх, формат BGR)
        std::vector<uint8_t> row(static_cast<size_t>(row_size), 0); // Инициализируем нулями для выравнивания
        
        for (int y = height - 1; y >= 0; --y)
        {
            // Копируем и конвертируем RGB в BGR
            for (int x = 0; x < width; ++x)
            {
                const size_t src_offset = (static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * static_cast<size_t>(channels);
                const size_t dst_offset = static_cast<size_t>(x) * 3;
                
                // Конвертируем RGB в BGR
                if (channels >= 3)
                {
                    row[dst_offset + 0] = data[src_offset + 2]; // B
                    row[dst_offset + 1] = data[src_offset + 1]; // G
                    row[dst_offset + 2] = data[src_offset + 0]; // R
                }
                else
                {
                    // Если меньше 3 каналов, используем градации серого
                    uint8_t gray = channels == 1 ? data[src_offset] : 128;
                    row[dst_offset + 0] = gray; // B
                    row[dst_offset + 1] = gray; // G
                    row[dst_offset + 2] = gray; // R
                }
            }
            
            file.write(reinterpret_cast<const char*>(row.data()), row_size);
            if (!file.good())
            {
                return false;
            }
        }

        return true;
    }
}

