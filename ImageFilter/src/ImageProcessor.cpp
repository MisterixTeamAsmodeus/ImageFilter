#include <ImageProcessor.h>
#include <utils/ImageLoader.h>
#include <utils/ImageSaver.h>
#include <utils/ImageConverter.h>
#include <utils/FilterResult.h>
#include <utils/SafeMath.h>

// STB Image - заголовочные файлы для работы с изображениями (только для stbi_image_free)
#include <stb_image.h>

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>

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

FilterResult ImageProcessor::loadFromFile(const std::string& filename, bool preserve_alpha)
{
    // Освобождаем предыдущие данные, если они были загружены
    if (data_ != nullptr)
    {
        stbi_image_free(data_);
        width_ = 0;
        height_ = 0;
        channels_ = 0;
    }

    // Используем ImageLoader для загрузки изображения
    ImageLoader::LoadedImage loaded;
    const auto result = ImageLoader::loadFromFile(filename, preserve_alpha, loaded);
    
    if (!result.isSuccess())
    {
        return result;
    }

    // Устанавливаем загруженные данные
    data_ = loaded.data;
    width_ = loaded.width;
    height_ = loaded.height;
    channels_ = loaded.channels;

    return FilterResult::success();
}

FilterResult ImageProcessor::saveToFile(const std::string& filename, bool preserve_alpha) const
{
    if (!isValid())
    {
        ErrorContext ctx = ErrorContext::withFilename(filename);
        return FilterResult::failure(FilterError::InvalidImage, 
                                   "Изображение не загружено", ctx);
    }

    // Используем ImageSaver для сохранения изображения
    return ImageSaver::saveToFile(filename, data_, width_, height_, channels_, 
                                 preserve_alpha, jpeg_quality_);
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

FilterResult ImageProcessor::convertToRGB()
{
    if (!isValid() || channels_ != 4)
    {
        ErrorContext ctx = ErrorContext::withImage(width_, height_, channels_);
        return FilterResult::failure(FilterError::InvalidImage, 
                                   "Изображение не загружено или не является RGBA", ctx);
    }

    if (width_ <= 0 || height_ <= 0)
    {
        ErrorContext ctx = ErrorContext::withImage(width_, height_, channels_);
        return FilterResult::failure(FilterError::InvalidSize, 
                                   "Некорректный размер изображения", ctx);
    }
    
    // Проверяем, что произведение не приведет к переполнению
    size_t width_height_product = 0;
    if (!SafeMath::safeMultiply(static_cast<size_t>(width_), static_cast<size_t>(height_), width_height_product))
    {
        ErrorContext ctx = ErrorContext::withImage(width_, height_, channels_);
        return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                   "Размер изображения слишком большой", ctx);
    }
    
    size_t rgb_size = 0;
    if (!SafeMath::safeMultiply(width_height_product, static_cast<size_t>(3), rgb_size))
    {
        ErrorContext ctx = ErrorContext::withImage(width_, height_, channels_);
        return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                   "Размер изображения слишком большой", ctx);
    }
    
    auto* rgb_data = static_cast<uint8_t*>(std::malloc(rgb_size));
    if (rgb_data == nullptr)
    {
        const int errno_code = errno;
        ErrorContext ctx = ErrorContext::withImage(width_, height_, channels_);
        if (errno_code != 0)
        {
            ctx.system_error_code = errno_code;
        }
        return FilterResult::failure(FilterError::OutOfMemory, 
                                   "Недостаточно памяти для преобразования RGBA в RGB", ctx);
    }

    // Используем ImageConverter для преобразования RGBA в RGB
    const auto convert_result = ImageConverter::convertRGBAToRGB(data_, width_, height_, rgb_data);
    if (!convert_result.isSuccess())
    {
        std::free(rgb_data);
        return convert_result;
    }

    // Освобождаем старые данные
    stbi_image_free(data_);

    // Устанавливаем новые данные
    data_ = rgb_data;
    channels_ = 3;

    return FilterResult::success();
}

FilterResult ImageProcessor::resize(int new_width, int new_height, const uint8_t* new_data)
{
    return resize(new_width, new_height, channels_ == 0 ? 3 : channels_, new_data);
}

FilterResult ImageProcessor::resize(int new_width, int new_height, int new_channels, const uint8_t* new_data)
{
    if (new_width <= 0 || new_height <= 0 || (new_channels != 3 && new_channels != 4))
    {
        ErrorContext ctx = ErrorContext::withImage(new_width, new_height, new_channels);
        return FilterResult::failure(FilterError::InvalidSize, 
                                   "Некорректный размер или количество каналов", ctx);
    }

    channels_ = new_channels;

    if (new_data == nullptr)
    {
        // Просто освобождаем старое изображение и устанавливаем новые размеры
        stbi_image_free(data_);
        data_ = nullptr;
        width_ = new_width;
        height_ = new_height;
        return FilterResult::success();
    }

    // Проверка на переполнение при вычислении размера
    size_t width_height_product = 0;
    if (!SafeMath::safeMultiply(static_cast<size_t>(new_width), static_cast<size_t>(new_height), width_height_product))
    {
        ErrorContext ctx = ErrorContext::withImage(new_width, new_height, new_channels);
        return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                   "Размер изображения слишком большой", ctx);
    }

    size_t new_size = 0;
    if (!SafeMath::safeMultiply(width_height_product, static_cast<size_t>(new_channels), new_size))
    {
        ErrorContext ctx = ErrorContext::withImage(new_width, new_height, new_channels);
        return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                   "Размер изображения слишком большой", ctx);
    }

    // Выделяем новую память через malloc (совместимо с stbi_image_free)
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(new_size));
    if (allocated_data == nullptr)
    {
        const int errno_code = errno;
        ErrorContext ctx = ErrorContext::withImage(new_width, new_height, new_channels);
        if (errno_code != 0)
        {
            ctx.system_error_code = errno_code;
        }
        return FilterResult::failure(FilterError::OutOfMemory, 
                                   "Недостаточно памяти для изменения размера", ctx);
    }

    // Копируем данные из переданного буфера
    std::memcpy(allocated_data, new_data, new_size);

    // Освобождаем старые данные
    stbi_image_free(data_);

    // Устанавливаем новые данные и размеры
    data_ = allocated_data;
    width_ = new_width;
    height_ = new_height;

    return FilterResult::success();
}

