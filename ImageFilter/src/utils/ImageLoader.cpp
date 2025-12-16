#include <utils/ImageLoader.h>
#include <utils/PathValidator.h>
#include <utils/BMPHandler.h>
#include <utils/FilterResult.h>
#include <utils/SafeMath.h>
#include <utils/ColorSpaceConverter.h>

// STB Image - заголовочные файлы для работы с изображениями
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

FilterResult ImageLoader::loadFromFile(const std::string& filename, 
                                       bool preserve_alpha, 
                                       LoadedImage& result)
{
    try
    {
        // Инициализируем результат
        result.data = nullptr;
        result.width = 0;
        result.height = 0;
        result.channels = 0;

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

        // Нормализация и валидация пути (проверяем до проверки размера файла)
        const std::string normalized_path = PathValidator::normalizeAndValidate(filename);
        if (normalized_path.empty())
        {
            ErrorContext ctx = ErrorContext::withFilename(filename);
            return FilterResult::failure(FilterError::InvalidFilePath, 
                                       "Небезопасный путь", ctx);
        }

        // Проверка размера файла перед загрузкой
        const uint64_t file_size = PathValidator::getFileSize(normalized_path);
        if (file_size == 0)
        {
            ErrorContext ctx = ErrorContext::withFilename(filename);
            return FilterResult::failure(FilterError::FileReadError, 
                                       "Не удалось определить размер файла", ctx);
        }

        if (file_size > PathValidator::DEFAULT_MAX_IMAGE_SIZE)
        {
            ErrorContext ctx = ErrorContext::withFilename(filename);
            return FilterResult::failure(FilterError::FileTooLarge, 
                                       "Файл слишком большой (" + 
                                       std::to_string(file_size) + " байт, максимум " + 
                                       std::to_string(PathValidator::DEFAULT_MAX_IMAGE_SIZE) + ")",
                                       ctx);
        }

        // Проверяем расширение файла для определения формата
        const auto dot_pos = filename.find_last_of('.');
        std::string extension;
        if (dot_pos != std::string::npos && dot_pos < normalized_path.length() - 1)
        {
            extension = normalized_path.substr(dot_pos + 1);
            std::transform(extension.begin(), extension.end(), extension.begin(),
                           [](unsigned char c) { return std::tolower(c); });
        }
        
        // Если это BMP файл, используем наш обработчик
        if (extension == "bmp")
        {
            int loaded_channels = 0;
            result.data = BMPHandler::loadBMP(normalized_path, result.width, result.height, loaded_channels);
            
            if (result.data == nullptr)
            {
                result.width = 0;
                result.height = 0;
                result.channels = 0;
                ErrorContext ctx = ErrorContext::withFilename(normalized_path);
                return FilterResult::failure(FilterError::FileReadError, 
                                           "Ошибка загрузки BMP изображения", ctx);
            }
            
            // BMP всегда загружается как RGB (3 канала)
            result.channels = 3;
            
            // Если нужен RGBA, преобразуем
            if (preserve_alpha)
            {
                // Проверка на переполнение с использованием SafeMath
                size_t width_height_product = 0;
                if (!SafeMath::safeMultiply(static_cast<size_t>(result.width), static_cast<size_t>(result.height), width_height_product))
                {
                    std::free(result.data);
                    result.data = nullptr;
                    result.width = 0;
                    result.height = 0;
                    result.channels = 0;
                    ErrorContext ctx = ErrorContext::withFilename(normalized_path);
                    ctx.image_width = result.width;
                    ctx.image_height = result.height;
                    ctx.image_channels = result.channels;
                    return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                               "Размер изображения слишком большой", ctx);
                }
                
                size_t rgba_size = 0;
                if (!SafeMath::safeMultiply(width_height_product, static_cast<size_t>(4), rgba_size))
                {
                    std::free(result.data);
                    result.data = nullptr;
                    result.width = 0;
                    result.height = 0;
                    result.channels = 0;
                    ErrorContext ctx = ErrorContext::withFilename(normalized_path);
                    ctx.image_width = result.width;
                    ctx.image_height = result.height;
                    ctx.image_channels = result.channels;
                    return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                               "Размер изображения слишком большой", ctx);
                }
                
                auto* rgba_data = static_cast<uint8_t*>(std::malloc(rgba_size));
                if (rgba_data == nullptr)
                {
                    std::free(result.data);
                    result.data = nullptr;
                    result.width = 0;
                    result.height = 0;
                    result.channels = 0;
                    const int errno_code = errno;
                    ErrorContext ctx = ErrorContext::withFilename(normalized_path);
                    ctx.image_width = result.width;
                    ctx.image_height = result.height;
                    ctx.image_channels = result.channels;
                    if (errno_code != 0)
                    {
                        ctx.system_error_code = errno_code;
                    }
                    return FilterResult::failure(FilterError::OutOfMemory, 
                                               "Недостаточно памяти для преобразования RGB в RGBA",
                                               ctx);
                }
                
                const auto convert_result = ColorSpaceConverter::convertRGBToRGBA(result.data, result.width, result.height, rgba_data);
                if (!convert_result.isSuccess())
                {
                    std::free(rgba_data);
                    std::free(result.data);
                    result.data = nullptr;
                    result.width = 0;
                    result.height = 0;
                    result.channels = 0;
                    return convert_result;
                }
                
                std::free(result.data);
                result.data = rgba_data;
                result.channels = 4;
            }
            
            return FilterResult::success();
        }
        
        // Для других форматов используем STB
        // STB автоматически определяет формат изображения по расширению файла
        // stbi_load возвращает указатель на данные или nullptr при ошибке
        // Последний параметр - желаемое количество каналов:
        // 0 = как в файле, 3 = RGB, 4 = RGBA
        const int desired_channels = preserve_alpha ? 4 : 3;
        int original_channels = 0;
        result.data = stbi_load(normalized_path.c_str(), &result.width, &result.height, &original_channels,
                          desired_channels);

        if (result.data == nullptr)
        {
            const char* stbi_reason = stbi_failure_reason();
            std::string error_msg = "Ошибка загрузки изображения";
            if (stbi_reason != nullptr)
            {
                error_msg += ": " + std::string(stbi_reason);
            }
            
            // Проверяем системную ошибку (errno)
            const int errno_code = errno;
            ErrorContext ctx = ErrorContext::withFilename(normalized_path);
            if (errno_code != 0)
            {
                ctx.system_error_code = errno_code;
            }
            
            return FilterResult::failure(FilterError::FileReadError, error_msg, ctx);
        }

        // Валидация загруженных данных
        if (result.width <= 0 || result.height <= 0)
        {
            stbi_image_free(result.data);
            result.data = nullptr;
            result.width = 0;
            result.height = 0;
            result.channels = 0;
            ErrorContext ctx = ErrorContext::withFilename(normalized_path);
            ctx.image_width = result.width;
            ctx.image_height = result.height;
            ctx.image_channels = original_channels;
            return FilterResult::failure(FilterError::InvalidSize, 
                                       "Некорректный размер изображения", ctx);
        }

        if (original_channels <= 0 || original_channels > 4)
        {
            stbi_image_free(result.data);
            result.data = nullptr;
            result.width = 0;
            result.height = 0;
            result.channels = 0;
            ErrorContext ctx = ErrorContext::withFilename(normalized_path);
            ctx.image_width = result.width;
            ctx.image_height = result.height;
            ctx.image_channels = original_channels;
            return FilterResult::failure(FilterError::InvalidChannels, 
                                       "Некорректное количество каналов: " + 
                                       std::to_string(original_channels), ctx);
        }

        // Устанавливаем количество каналов в зависимости от запроса
        // Если preserve_alpha = true и файл имеет альфа-канал, будет 4 канала
        // Если preserve_alpha = false или файл не имеет альфа-канала, будет 3 канала
        result.channels = desired_channels;

        return FilterResult::success();
    }
    catch (const std::bad_alloc& e)
    {
        if (result.data != nullptr)
        {
            stbi_image_free(result.data);
            result.data = nullptr;
        }
        result.width = 0;
        result.height = 0;
        result.channels = 0;
        ErrorContext ctx = ErrorContext::withFilename(filename);
        return FilterResult::failure(FilterError::OutOfMemory, 
                                   "Недостаточно памяти: " + std::string(e.what()), ctx);
    }
    catch (const std::exception& e)
    {
        if (result.data != nullptr)
        {
            stbi_image_free(result.data);
            result.data = nullptr;
        }
        result.width = 0;
        result.height = 0;
        result.channels = 0;
        ErrorContext ctx = ErrorContext::withFilename(filename);
        return FilterResult::failure(FilterError::SystemError, 
                                   "Исключение: " + std::string(e.what()), ctx);
    }
    catch (...)
    {
        if (result.data != nullptr)
        {
            stbi_image_free(result.data);
            result.data = nullptr;
        }
        result.width = 0;
        result.height = 0;
        result.channels = 0;
        ErrorContext ctx = ErrorContext::withFilename(filename);
        return FilterResult::failure(FilterError::SystemError, 
                                   "Неизвестное исключение", ctx);
    }
}

