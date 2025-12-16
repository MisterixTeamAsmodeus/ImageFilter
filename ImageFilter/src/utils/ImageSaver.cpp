#include <utils/ImageSaver.h>
#include <utils/PathValidator.h>
#include <utils/BMPHandler.h>
#include <utils/FilterResult.h>
#include <utils/SafeMath.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <stdexcept>

namespace
{
    /**
     * @brief Преобразует RGBA данные в RGB данные для сохранения
     * @param rgba_data Исходные RGBA данные
     * @param width Ширина изображения
     * @param height Высота изображения
     * @param rgb_data Выходной буфер для RGB данных (должен быть выделен)
     * @return FilterResult с результатом операции
     */
    FilterResult convertRGBAToRGBForSave(const uint8_t* rgba_data, 
                                          int width, 
                                          int height, 
                                          uint8_t* rgb_data)
    {
        size_t width_height_product = 0;
        if (!SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), width_height_product))
        {
            ErrorContext ctx = ErrorContext::withImage(width, height, 4);
            return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                       "Размер изображения слишком большой", ctx);
        }
        
        for (size_t i = 0; i < width_height_product; ++i)
        {
            size_t rgb_offset = 0;
            size_t rgba_offset = 0;
            if (!SafeMath::safeMultiply(i, static_cast<size_t>(3), rgb_offset) ||
                !SafeMath::safeMultiply(i, static_cast<size_t>(4), rgba_offset))
            {
                ErrorContext ctx = ErrorContext::withImage(width, height, 4);
                return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                           "Размер изображения слишком большой", ctx);
            }
            
            rgb_data[rgb_offset + 0] = rgba_data[rgba_offset + 0]; // R
            rgb_data[rgb_offset + 1] = rgba_data[rgba_offset + 1]; // G
            rgb_data[rgb_offset + 2] = rgba_data[rgba_offset + 2]; // B
        }
        
        return FilterResult::success();
    }

    /**
     * @brief Преобразует RGBA данные в RGB данные построчно (для JPEG)
     * @param rgba_data Исходные RGBA данные
     * @param width Ширина изображения
     * @param height Высота изображения
     * @param rgb_data Выходной буфер для RGB данных (должен быть выделен)
     * @return FilterResult с результатом операции
     */
    FilterResult convertRGBAToRGBRowByRow(const uint8_t* rgba_data, 
                                           int width, 
                                           int height, 
                                           uint8_t* rgb_data)
    {
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                size_t y_width_sum = 0;
                size_t pixel_index = 0;
                if (!SafeMath::safeMultiply(static_cast<size_t>(y), static_cast<size_t>(width), y_width_sum) ||
                    !SafeMath::safeAdd(y_width_sum, static_cast<size_t>(x), pixel_index))
                {
                    ErrorContext ctx = ErrorContext::withImage(width, height, 4);
                    return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                               "Размер изображения слишком большой", ctx);
                }
                
                size_t rgba_offset = 0;
                size_t rgb_offset = 0;
                if (!SafeMath::safeMultiply(pixel_index, static_cast<size_t>(4), rgba_offset) ||
                    !SafeMath::safeMultiply(pixel_index, static_cast<size_t>(3), rgb_offset))
                {
                    ErrorContext ctx = ErrorContext::withImage(width, height, 4);
                    return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                               "Размер изображения слишком большой", ctx);
                }
                
                rgb_data[rgb_offset + 0] = rgba_data[rgba_offset + 0]; // R
                rgb_data[rgb_offset + 1] = rgba_data[rgba_offset + 1]; // G
                rgb_data[rgb_offset + 2] = rgba_data[rgba_offset + 2]; // B
            }
        }
        
        return FilterResult::success();
    }
}

FilterResult ImageSaver::saveToFile(const std::string& filename,
                                    const uint8_t* data,
                                    int width,
                                    int height,
                                    int channels,
                                    bool preserve_alpha,
                                    int jpeg_quality)
{
    try
    {
        if (filename.empty())
        {
            ErrorContext ctx = ErrorContext::withFilename(filename);
            return FilterResult::failure(FilterError::InvalidFilePath, 
                                       "Путь к файлу пуст", ctx);
        }

        // Валидация пути к файлу
        if (PathValidator::containsDangerousCharacters(filename))
        {
            ErrorContext ctx = ErrorContext::withFilename(filename);
            return FilterResult::failure(FilterError::InvalidFilePath, 
                                       "Путь содержит опасные символы", ctx);
        }

        // Нормализация пути
        const std::string normalized_path = PathValidator::normalizeAndValidate(filename);
        if (normalized_path.empty())
        {
            ErrorContext ctx = ErrorContext::withFilename(filename);
            return FilterResult::failure(FilterError::InvalidFilePath, 
                                       "Небезопасный путь", ctx);
        }

        // Валидация данных изображения
        if (data == nullptr)
        {
            ErrorContext ctx = ErrorContext::withFilename(filename);
            return FilterResult::failure(FilterError::InvalidImage, 
                                       "Данные изображения не заданы", ctx);
        }

        // Валидация размеров изображения
        if (width <= 0 || height <= 0)
        {
            ErrorContext ctx = ErrorContext::withFilename(filename);
            ctx.image_width = width;
            ctx.image_height = height;
            ctx.image_channels = channels;
            return FilterResult::failure(FilterError::InvalidSize, 
                                       "Некорректный размер изображения", ctx);
        }

        // Валидация каналов изображения
        if (channels != 3 && channels != 4)
        {
            ErrorContext ctx = ErrorContext::withFilename(filename);
            ctx.image_width = width;
            ctx.image_height = height;
            ctx.image_channels = channels;
            return FilterResult::failure(FilterError::InvalidChannels, 
                                       "Ожидается 3 канала (RGB) или 4 канала (RGBA), получено: " + std::to_string(channels), ctx);
        }

        // Определяем формат по расширению файла
        const auto dot_pos = normalized_path.find_last_of('.');
        if (dot_pos == std::string::npos || dot_pos == normalized_path.length() - 1)
        {
            ErrorContext ctx = ErrorContext::withFilename(normalized_path);
            return FilterResult::failure(FilterError::InvalidFilePath, 
                                       "Некорректный путь к файлу (отсутствует расширение)", ctx);
        }

        auto extension = normalized_path.substr(dot_pos + 1);

        // Преобразуем расширение в нижний регистр для сравнения
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        
        // Проверяем поддержку BMP формата
        if (extension == "bmp")
        {
            // Преобразуем RGBA в RGB, если нужно (BMP не поддерживает альфа-канал)
            const uint8_t* save_data = data;
            std::unique_ptr<uint8_t[]> rgb_data_ptr;
            int save_channels = channels;
            
            if (channels == 4)
            {
                // Преобразуем RGBA в RGB
                size_t width_height_product = 0;
                if (!SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), width_height_product))
                {
                    ErrorContext ctx = ErrorContext::withFilename(normalized_path);
                    ctx.image_width = width;
                    ctx.image_height = height;
                    ctx.image_channels = channels;
                    return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                               "Размер изображения слишком большой", ctx);
                }
                
                size_t rgb_size = 0;
                if (!SafeMath::safeMultiply(width_height_product, static_cast<size_t>(3), rgb_size))
                {
                    ErrorContext ctx = ErrorContext::withFilename(normalized_path);
                    ctx.image_width = width;
                    ctx.image_height = height;
                    ctx.image_channels = channels;
                    return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                               "Размер изображения слишком большой", ctx);
                }
                
                rgb_data_ptr = std::make_unique<uint8_t[]>(rgb_size);
                uint8_t* rgb_data = rgb_data_ptr.get();
                
                const auto convert_result = convertRGBAToRGBForSave(data, width, height, rgb_data);
                if (!convert_result.isSuccess())
                {
                    return convert_result;
                }
                
                save_data = rgb_data;
                save_channels = 3;
            }
            
            const bool bmp_result = BMPHandler::saveBMP(normalized_path, width, height, save_channels, save_data);
            if (!bmp_result)
            {
                ErrorContext ctx = ErrorContext::withFilename(normalized_path);
                ctx.image_width = width;
                ctx.image_height = height;
                ctx.image_channels = save_channels;
                return FilterResult::failure(FilterError::FileWriteError, 
                                           "Ошибка сохранения BMP изображения", ctx);
            }
            return FilterResult::success();
        }

        int result = 0;
        int save_channels = channels;

        // Если нужно сохранить как RGB, но изображение RGBA, нужно преобразовать
        if (!preserve_alpha && channels == 4)
        {
            // JPEG не поддерживает альфа-канал, всегда используем 3 канала
            if (extension == "jpg" || extension == "jpeg")
            {
                save_channels = 3;
            }
        }
        // Если нужно сохранить альфа-канал, но изображение RGB, используем текущее количество каналов
        else if (preserve_alpha && channels == 3)
        {
            // Изображение RGB, альфа-канал недоступен
            save_channels = 3;
        }

        if (extension == "jpg" || extension == "jpeg")
        {
            // JPEG не поддерживает альфа-канал, всегда сохраняем как RGB
            // Если изображение RGBA, нужно преобразовать
            if (channels == 4)
            {
                // Создаем временный буфер для RGB данных
                size_t width_height_product = 0;
                if (!SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), width_height_product))
                {
                    ErrorContext ctx = ErrorContext::withFilename(filename);
                    ctx.image_width = width;
                    ctx.image_height = height;
                    ctx.image_channels = channels;
                    return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                               "Размер изображения слишком большой", ctx);
                }
                
                size_t rgb_size = 0;
                if (!SafeMath::safeMultiply(width_height_product, static_cast<size_t>(3), rgb_size))
                {
                    ErrorContext ctx = ErrorContext::withFilename(filename);
                    ctx.image_width = width;
                    ctx.image_height = height;
                    ctx.image_channels = channels;
                    return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                               "Размер изображения слишком большой", ctx);
                }
                
                auto* rgb_data = static_cast<uint8_t*>(std::malloc(rgb_size));
                if (rgb_data == nullptr)
                {
                    const int errno_code = errno;
                    ErrorContext ctx = ErrorContext::withFilename(filename);
                    ctx.image_width = width;
                    ctx.image_height = height;
                    ctx.image_channels = channels;
                    if (errno_code != 0)
                    {
                        ctx.system_error_code = errno_code;
                    }
                    return FilterResult::failure(FilterError::OutOfMemory, 
                                               "Недостаточно памяти для преобразования RGBA в RGB",
                                               ctx);
                }

                // Преобразуем RGBA в RGB, игнорируя альфа-канал
                const auto convert_result = convertRGBAToRGBRowByRow(data, width, height, rgb_data);
                if (!convert_result.isSuccess())
                {
                    std::free(rgb_data);
                    return convert_result;
                }

                result = stbi_write_jpg(normalized_path.c_str(), width, height, 3, rgb_data, jpeg_quality);
                std::free(rgb_data);
            }
            else
            {
                // Сохраняем как JPEG с настраиваемым качеством (0-100, где 100 - наилучшее качество)
                result = stbi_write_jpg(normalized_path.c_str(), width, height, 3, data, jpeg_quality);
            }
        }
        else if (extension == "png")
        {
            // PNG поддерживает альфа-канал
            // Сохраняем с текущим количеством каналов (3 или 4)
            // stride_in_bytes = 0 означает автоматический расчет шага (width * channels)
            result = stbi_write_png(normalized_path.c_str(), width, height, save_channels,
                                    data, 0);
        }
        else
        {
            ErrorContext ctx = ErrorContext::withFilename(normalized_path);
            return FilterResult::failure(FilterError::UnsupportedFormat, 
                                       "Неподдерживаемый формат файла: " + extension, ctx);
        }

        if (result == 0)
        {
            const int errno_code = errno;
            ErrorContext ctx = ErrorContext::withFilename(normalized_path);
            if (errno_code != 0)
            {
                ctx.system_error_code = errno_code;
            }
            return FilterResult::failure(FilterError::FileWriteError, 
                                       "Ошибка записи файла", ctx);
        }

        return FilterResult::success();
    }
    catch (const std::bad_alloc& e)
    {
        ErrorContext ctx = ErrorContext::withFilename(filename);
        return FilterResult::failure(FilterError::OutOfMemory, 
                                   "Недостаточно памяти: " + std::string(e.what()), ctx);
    }
    catch (const std::exception& e)
    {
        ErrorContext ctx = ErrorContext::withFilename(filename);
        return FilterResult::failure(FilterError::SystemError, 
                                   "Исключение: " + std::string(e.what()), ctx);
    }
    catch (...)
    {
        ErrorContext ctx = ErrorContext::withFilename(filename);
        return FilterResult::failure(FilterError::SystemError, 
                                   "Неизвестное исключение", ctx);
    }
}

