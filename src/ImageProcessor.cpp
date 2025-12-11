#include <ImageProcessor.h>
#include <utils/Logger.h>
#include <utils/PathValidator.h>
#include <utils/BMPHandler.h>

// STB Image - заголовочные файлы для работы с изображениями
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

ImageProcessor::~ImageProcessor()
{
    // Освобождаем память, выделенную STB
    stbi_image_free(data_);
}

ImageProcessor::ImageProcessor(ImageProcessor&& other) noexcept
    : data_(other.data_)
    , width_(other.width_)
    , height_(other.height_)
    , channels_(other.channels_)
    , jpeg_quality_(other.jpeg_quality_)
{
    // Обнуляем данные в исходном объекте, чтобы деструктор не освободил память
    other.data_ = nullptr;
    other.width_ = 0;
    other.height_ = 0;
    other.channels_ = 0;
    other.jpeg_quality_ = 90;
}

ImageProcessor& ImageProcessor::operator=(ImageProcessor&& other) noexcept
{
    if (this != &other)
    {
        // Освобождаем текущие данные
        stbi_image_free(data_);
        
        // Переносим данные из другого объекта
        data_ = other.data_;
        width_ = other.width_;
        height_ = other.height_;
        channels_ = other.channels_;
        jpeg_quality_ = other.jpeg_quality_;
        
        // Обнуляем данные в исходном объекте
        other.data_ = nullptr;
        other.width_ = 0;
        other.height_ = 0;
        other.channels_ = 0;
        other.jpeg_quality_ = 90;
    }
    
    return *this;
}

bool ImageProcessor::loadFromFile(const std::string& filename, bool preserve_alpha)
{
    try
    {
        if (filename.empty())
        {
            Logger::error("Ошибка загрузки изображения: путь к файлу пуст");
            return false;
        }

        // Валидация пути к файлу
        if (PathValidator::containsDangerousCharacters(filename))
        {
            Logger::error("Ошибка загрузки изображения: путь содержит опасные символы: " + filename);
            return false;
        }

        // Проверка размера файла перед загрузкой
        const uint64_t file_size = PathValidator::getFileSize(filename);
        if (file_size == 0)
        {
            Logger::error("Ошибка загрузки изображения: не удалось определить размер файла: " + filename);
            return false;
        }

        if (file_size > PathValidator::DEFAULT_MAX_IMAGE_SIZE)
        {
            Logger::error("Ошибка загрузки изображения: файл слишком большой (" + 
                         std::to_string(file_size) + " байт, максимум " + 
                         std::to_string(PathValidator::DEFAULT_MAX_IMAGE_SIZE) + "): " + filename);
            return false;
        }

        // Нормализация и валидация пути
        const std::string normalized_path = PathValidator::normalizeAndValidate(filename);
        if (normalized_path.empty())
        {
            Logger::error("Ошибка загрузки изображения: небезопасный путь: " + filename);
            return false;
        }

        if (data_ != nullptr) {
            // Освобождаем предыдущие данные, если они были загружены
            stbi_image_free(data_);
            width_ = 0;
            height_ = 0;
            channels_ = 0;
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
            data_ = BMPHandler::loadBMP(normalized_path, width_, height_, loaded_channels);
            
            if (data_ == nullptr)
            {
                Logger::error("Ошибка загрузки BMP изображения: " + normalized_path);
                width_ = 0;
                height_ = 0;
                channels_ = 0;
                return false;
            }
            
            // BMP всегда загружается как RGB (3 канала)
            channels_ = 3;
            
            // Если нужен RGBA, преобразуем
            if (preserve_alpha)
            {
                // Проверка на переполнение
                const size_t max_safe_size = std::numeric_limits<size_t>::max() / 4;
                if (static_cast<size_t>(width_) > max_safe_size / static_cast<size_t>(height_))
                {
                    std::free(data_);
                    data_ = nullptr;
                    width_ = 0;
                    height_ = 0;
                    channels_ = 0;
                    Logger::error("Ошибка загрузки BMP изображения: размер слишком большой");
                    return false;
                }
                
                const size_t rgba_size = static_cast<size_t>(width_) * static_cast<size_t>(height_) * 4;
                auto* rgba_data = static_cast<uint8_t*>(std::malloc(rgba_size));
                if (rgba_data == nullptr)
                {
                    std::free(data_);
                    data_ = nullptr;
                    width_ = 0;
                    height_ = 0;
                    channels_ = 0;
                    return false;
                }
                
                for (int i = 0; i < width_ * height_; ++i)
                {
                    rgba_data[i * 4 + 0] = data_[i * 3 + 0]; // R
                    rgba_data[i * 4 + 1] = data_[i * 3 + 1]; // G
                    rgba_data[i * 4 + 2] = data_[i * 3 + 2]; // B
                    rgba_data[i * 4 + 3] = 255; // A (полная непрозрачность)
                }
                
                std::free(data_);
                data_ = rgba_data;
                channels_ = 4;
            }
            
            return true;
        }
        
        // Для других форматов используем STB
        // STB автоматически определяет формат изображения по расширению файла
        // stbi_load возвращает указатель на данные или nullptr при ошибке
        // Последний параметр - желаемое количество каналов:
        // 0 = как в файле, 3 = RGB, 4 = RGBA
        const int desired_channels = preserve_alpha ? 4 : 3;
        int original_channels = 0;
        data_ = stbi_load(normalized_path.c_str(), &width_, &height_, &original_channels,
                          desired_channels);

        if (data_ == nullptr)
        {
            const char* stbi_reason = stbi_failure_reason();
            std::string error_msg = "Ошибка загрузки изображения: " + normalized_path;
            if (stbi_reason != nullptr)
            {
                error_msg += ". Причина: " + std::string(stbi_reason);
            }
            
            // Проверяем системную ошибку (errno)
            const int errno_code = errno;
            if (errno_code != 0)
            {
                error_msg += ". Системная ошибка: " + std::to_string(errno_code);
                const char* errno_str = std::strerror(errno_code);
                if (errno_str != nullptr)
                {
                    error_msg += " (" + std::string(errno_str) + ")";
                }
            }
            
            Logger::error(error_msg);
            return false;
        }

        // Валидация загруженных данных
        if (width_ <= 0 || height_ <= 0)
        {
            Logger::error("Ошибка загрузки изображения: " + normalized_path + 
                         ". Некорректный размер: " + std::to_string(width_) + 
                         "x" + std::to_string(height_));
            stbi_image_free(data_);
            data_ = nullptr;
            width_ = 0;
            height_ = 0;
            channels_ = 0;
            return false;
        }

        if (original_channels <= 0 || original_channels > 4)
        {
            Logger::error("Ошибка загрузки изображения: " + filename + 
                         ". Некорректное количество каналов: " + std::to_string(original_channels));
            stbi_image_free(data_);
            data_ = nullptr;
            width_ = 0;
            height_ = 0;
            channels_ = 0;
            return false;
        }

        // Устанавливаем количество каналов в зависимости от запроса
        // Если preserve_alpha = true и файл имеет альфа-канал, будет 4 канала
        // Если preserve_alpha = false или файл не имеет альфа-канала, будет 3 канала
        channels_ = desired_channels;

        return true;
    }
    catch (const std::bad_alloc& e)
    {
        Logger::error("Ошибка загрузки изображения: " + filename + 
                     ". Недостаточно памяти: " + std::string(e.what()));
        if (data_ != nullptr)
        {
            stbi_image_free(data_);
            data_ = nullptr;
        }
        width_ = 0;
        height_ = 0;
        channels_ = 0;
        return false;
    }
    catch (const std::exception& e)
    {
        Logger::error("Ошибка загрузки изображения: " + filename + 
                     ". Исключение: " + std::string(e.what()));
        if (data_ != nullptr)
        {
            stbi_image_free(data_);
            data_ = nullptr;
        }
        width_ = 0;
        height_ = 0;
        channels_ = 0;
        return false;
    }
    catch (...)
    {
        Logger::error("Ошибка загрузки изображения: " + filename + 
                     ". Неизвестное исключение");
        if (data_ != nullptr)
        {
            stbi_image_free(data_);
            data_ = nullptr;
        }
        width_ = 0;
        height_ = 0;
        channels_ = 0;
        return false;
    }
}

bool ImageProcessor::saveToFile(const std::string& filename, bool preserve_alpha) const
{
    try
    {
        if (filename.empty())
        {
            Logger::error("Ошибка сохранения изображения: путь к файлу пуст");
            return false;
        }

        // Валидация пути к файлу
        if (PathValidator::containsDangerousCharacters(filename))
        {
            Logger::error("Ошибка сохранения изображения: путь содержит опасные символы: " + filename);
            return false;
        }

        // Нормализация пути
        const std::string normalized_path = PathValidator::normalizeAndValidate(filename);
        if (normalized_path.empty())
        {
            Logger::error("Ошибка сохранения изображения: небезопасный путь: " + filename);
            return false;
        }

        if (!isValid())
        {
            Logger::error("Ошибка сохранения изображения: " + filename + 
                         ". Изображение не загружено");
            return false;
        }

        // Валидация размеров изображения
        if (width_ <= 0 || height_ <= 0)
        {
            Logger::error("Ошибка сохранения изображения: " + filename + 
                         ". Некорректный размер: " + std::to_string(width_) + 
                         "x" + std::to_string(height_));
            return false;
        }

        // Определяем формат по расширению файла
        const auto dot_pos = normalized_path.find_last_of('.');
        if (dot_pos == std::string::npos || dot_pos == normalized_path.length() - 1)
        {
            Logger::error("Ошибка сохранения изображения: " + normalized_path + 
                         ". Некорректный путь к файлу (отсутствует расширение)");
            return false;
        }

        auto extension = normalized_path.substr(dot_pos + 1);

        // Преобразуем расширение в нижний регистр для сравнения
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        
        // Проверяем поддержку BMP формата
        if (extension == "bmp")
        {
            // Преобразуем RGBA в RGB, если нужно (BMP не поддерживает альфа-канал)
            const uint8_t* save_data = data_;
            std::unique_ptr<uint8_t[]> rgb_data_ptr;
            int save_channels = channels_;
            
            if (channels_ == 4)
            {
                // Преобразуем RGBA в RGB
                const size_t rgb_size = static_cast<size_t>(width_) * static_cast<size_t>(height_) * 3;
                rgb_data_ptr = std::make_unique<uint8_t[]>(rgb_size);
                uint8_t* rgb_data = rgb_data_ptr.get();
                
                for (int i = 0; i < width_ * height_; ++i)
                {
                    rgb_data[i * 3 + 0] = data_[i * 4 + 0]; // R
                    rgb_data[i * 3 + 1] = data_[i * 4 + 1]; // G
                    rgb_data[i * 3 + 2] = data_[i * 4 + 2]; // B
                }
                
                save_data = rgb_data;
                save_channels = 3;
            }
            
            return BMPHandler::saveBMP(normalized_path, width_, height_, save_channels, save_data);
        }

        auto result = 0;
        int save_channels = channels_;

    // Если нужно сохранить как RGB, но изображение RGBA, нужно преобразовать
    if (!preserve_alpha && channels_ == 4)
    {
        // JPEG не поддерживает альфа-канал, всегда используем 3 канала
        if (extension == "jpg" || extension == "jpeg")
        {
            save_channels = 3;
        }
    }
    // Если нужно сохранить альфа-канал, но изображение RGB, используем текущее количество каналов
    else if (preserve_alpha && channels_ == 3)
    {
        // Изображение RGB, альфа-канал недоступен
        save_channels = 3;
    }

    if (extension == "jpg" || extension == "jpeg")
    {
        // JPEG не поддерживает альфа-канал, всегда сохраняем как RGB
        // Если изображение RGBA, нужно преобразовать
        if (channels_ == 4)
        {
            // Создаем временный буфер для RGB данных
            const auto rgb_size = static_cast<size_t>(width_) * static_cast<size_t>(height_) * 3U;
            auto* rgb_data = static_cast<uint8_t*>(std::malloc(rgb_size));
            if (rgb_data == nullptr)
            {
                const int errno_code = errno;
                std::string error_msg = "Ошибка сохранения изображения: " + filename + 
                                       ". Недостаточно памяти для преобразования RGBA в RGB";
                if (errno_code != 0)
                {
                    error_msg += ". Системная ошибка: " + std::to_string(errno_code);
                    const char* errno_str = std::strerror(errno_code);
                    if (errno_str != nullptr)
                    {
                        error_msg += " (" + std::string(errno_str) + ")";
                    }
                }
                Logger::error(error_msg);
                return false;
            }

            // Преобразуем RGBA в RGB, игнорируя альфа-канал
            for (int y = 0; y < height_; ++y)
            {
                for (int x = 0; x < width_; ++x)
                {
                    const auto rgba_offset = static_cast<size_t>(y * width_ + x) * 4;
                    const auto rgb_offset = static_cast<size_t>(y * width_ + x) * 3;
                    rgb_data[rgb_offset + 0] = data_[rgba_offset + 0]; // R
                    rgb_data[rgb_offset + 1] = data_[rgba_offset + 1]; // G
                    rgb_data[rgb_offset + 2] = data_[rgba_offset + 2]; // B
                    // Альфа-канал игнорируется
                }
            }

            result = stbi_write_jpg(normalized_path.c_str(), width_, height_, 3, rgb_data, jpeg_quality_);
            std::free(rgb_data);
        }
        else
        {
            // Сохраняем как JPEG с настраиваемым качеством (0-100, где 100 - наилучшее качество)
            result = stbi_write_jpg(normalized_path.c_str(), width_, height_, 3, data_, jpeg_quality_);
        }
    }
    else if (extension == "png")
    {
        // PNG поддерживает альфа-канал
        // Сохраняем с текущим количеством каналов (3 или 4)
        // stride_in_bytes = 0 означает автоматический расчет шага (width * channels)
        result = stbi_write_png(normalized_path.c_str(), width_, height_, save_channels,
                                data_, 0);
    }
    else
    {
        Logger::error("Ошибка сохранения изображения: " + normalized_path + 
                     ". Неподдерживаемый формат файла: " + extension);
        return false;
    }

        if (result == 0)
        {
            const int errno_code = errno;
            std::string error_msg = "Ошибка сохранения изображения: " + normalized_path;
            if (errno_code != 0)
            {
                error_msg += ". Системная ошибка: " + std::to_string(errno_code);
                const char* errno_str = std::strerror(errno_code);
                if (errno_str != nullptr)
                {
                    error_msg += " (" + std::string(errno_str) + ")";
                }
            }
            Logger::error(error_msg);
            return false;
        }

        return true;
    }
    catch (const std::bad_alloc& e)
    {
        Logger::error("Ошибка сохранения изображения: " + filename + 
                     ". Недостаточно памяти: " + std::string(e.what()));
        return false;
    }
    catch (const std::exception& e)
    {
        Logger::error("Ошибка сохранения изображения: " + filename + 
                     ". Исключение: " + std::string(e.what()));
        return false;
    }
    catch (...)
    {
        Logger::error("Ошибка сохранения изображения: " + filename + 
                     ". Неизвестное исключение");
        return false;
    }
}

int ImageProcessor::getWidth() const noexcept { return width_; }
int ImageProcessor::getHeight() const noexcept { return height_; }
int ImageProcessor::getChannels() const noexcept { return channels_; }
uint8_t* ImageProcessor::getData() noexcept { return data_; }
const uint8_t* ImageProcessor::getData() const noexcept { return data_; }
bool ImageProcessor::isValid() const noexcept { return data_ != nullptr; }

bool ImageProcessor::hasAlpha() const noexcept
{
    return channels_ == 4;
}

bool ImageProcessor::setJpegQuality(int quality)
{
    if (quality < 0 || quality > 100)
    {
        return false;
    }
    jpeg_quality_ = quality;
    return true;
}

int ImageProcessor::getJpegQuality() const noexcept
{
    return jpeg_quality_;
}

bool ImageProcessor::convertToRGB()
{
    try
    {
        if (!isValid() || channels_ != 4)
        {
            return false;
        }

        if (width_ <= 0 || height_ <= 0)
        {
            return false;
        }

        // Проверка на переполнение при вычислении размера
        if (width_ <= 0 || height_ <= 0)
        {
            return false;
        }
        
        // Проверяем, что произведение не приведет к переполнению
        const size_t max_safe_size = std::numeric_limits<size_t>::max() / 3;
        if (static_cast<size_t>(width_) > max_safe_size / static_cast<size_t>(height_))
        {
            Logger::error("Ошибка преобразования RGBA в RGB: размер изображения слишком большой");
            return false;
        }
        
        const auto rgb_size = static_cast<size_t>(width_) * static_cast<size_t>(height_) * 3U;
        auto* rgb_data = static_cast<uint8_t*>(std::malloc(rgb_size));
        if (rgb_data == nullptr)
        {
            const int errno_code = errno;
            if (errno_code != 0)
            {
                Logger::error("Ошибка преобразования RGBA в RGB: недостаточно памяти. " +
                             std::string(std::strerror(errno_code)));
            }
            return false;
        }

    // Преобразуем RGBA в RGB
    // Используем альфа-канал для композиции (alpha blending с белым фоном)
    for (int y = 0; y < height_; ++y)
    {
        for (int x = 0; x < width_; ++x)
        {
            const auto rgba_offset = static_cast<size_t>(y * width_ + x) * 4;
            const auto rgb_offset = static_cast<size_t>(y * width_ + x) * 3;
            
            const auto alpha = static_cast<float>(data_[rgba_offset + 3]) / 255.0f;
            
            // Композиция с белым фоном: result = alpha * color + (1 - alpha) * white
            rgb_data[rgb_offset + 0] = static_cast<uint8_t>(
                alpha * static_cast<float>(data_[rgba_offset + 0]) + (1.0f - alpha) * 255.0f);
            rgb_data[rgb_offset + 1] = static_cast<uint8_t>(
                alpha * static_cast<float>(data_[rgba_offset + 1]) + (1.0f - alpha) * 255.0f);
            rgb_data[rgb_offset + 2] = static_cast<uint8_t>(
                alpha * static_cast<float>(data_[rgba_offset + 2]) + (1.0f - alpha) * 255.0f);
        }
    }

    // Освобождаем старые данные
    stbi_image_free(data_);

        // Устанавливаем новые данные
        data_ = rgb_data;
        channels_ = 3;

        return true;
    }
    catch (const std::bad_alloc& e)
    {
        Logger::error("Ошибка преобразования RGBA в RGB: недостаточно памяти. " +
                     std::string(e.what()));
        return false;
    }
    catch (const std::exception& e)
    {
        Logger::error("Ошибка преобразования RGBA в RGB: " + std::string(e.what()));
        return false;
    }
    catch (...)
    {
        Logger::error("Ошибка преобразования RGBA в RGB: неизвестное исключение");
        return false;
    }
}

bool ImageProcessor::resize(int new_width, int new_height, uint8_t* new_data)
{
    if (new_width <= 0 || new_height <= 0)
    {
        return false;
    }

    if (channels_ == 0)
    {
        channels_ = 3; // Устанавливаем каналы по умолчанию
    }

    // Проверка на переполнение при вычислении размера
    const size_t max_safe_size = std::numeric_limits<size_t>::max() / static_cast<size_t>(channels_);
    if (static_cast<size_t>(new_width) > max_safe_size / static_cast<size_t>(new_height))
    {
        Logger::error("Ошибка изменения размера изображения: размер слишком большой");
        return false;
    }

    const auto new_size = static_cast<size_t>(new_width) * static_cast<size_t>(new_height) * static_cast<size_t>(channels_);

    if (new_data == nullptr)
    {
        // Просто освобождаем старое изображение и устанавливаем новые размеры
        stbi_image_free(data_);
        data_ = nullptr;
        width_ = new_width;
        height_ = new_height;
        return true;
    }

    // Выделяем новую память через malloc (совместимо с stbi_image_free)
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(new_size));
    if (allocated_data == nullptr)
    {
        return false;
    }

    // Копируем данные из переданного буфера
    std::memcpy(allocated_data, new_data, new_size);

    // Освобождаем старые данные
    stbi_image_free(data_);

    // Устанавливаем новые данные и размеры
    data_ = allocated_data;
    width_ = new_width;
    height_ = new_height;

    return true;
}

bool ImageProcessor::resize(int new_width, int new_height, int new_channels, uint8_t* new_data)
{
    if (new_width <= 0 || new_height <= 0 || (new_channels != 3 && new_channels != 4))
    {
        return false;
    }

    channels_ = new_channels;

    if (new_data == nullptr)
    {
        // Просто освобождаем старое изображение и устанавливаем новые размеры
        stbi_image_free(data_);
        data_ = nullptr;
        width_ = new_width;
        height_ = new_height;
        return true;
    }

    // Проверка на переполнение при вычислении размера
    const size_t max_safe_size = std::numeric_limits<size_t>::max() / static_cast<size_t>(new_channels);
    if (static_cast<size_t>(new_width) > max_safe_size / static_cast<size_t>(new_height))
    {
        Logger::error("Ошибка изменения размера изображения: размер слишком большой");
        return false;
    }

    const auto new_size = static_cast<size_t>(new_width) * static_cast<size_t>(new_height) * static_cast<size_t>(new_channels);

    // Выделяем новую память через malloc (совместимо с stbi_image_free)
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(new_size));
    if (allocated_data == nullptr)
    {
        return false;
    }

    // Копируем данные из переданного буфера
    std::memcpy(allocated_data, new_data, new_size);

    // Освобождаем старые данные
    stbi_image_free(data_);

    // Устанавливаем новые данные и размеры
    data_ = allocated_data;
    width_ = new_width;
    height_ = new_height;

    return true;
}
