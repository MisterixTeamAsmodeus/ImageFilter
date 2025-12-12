#include <ImageProcessor.h>

// STB Image - заголовочные файлы для работы с изображениями
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>

ImageProcessor::~ImageProcessor()
{
    // Освобождаем память, выделенную STB
    stbi_image_free(data_);
}

bool ImageProcessor::loadFromFile(const std::string& filename)
{
    // Освобождаем предыдущие данные, если они были загружены
    stbi_image_free(data_);
    width_ = 0;
    height_ = 0;
    channels_ = 0;

    // STB автоматически определяет формат изображения по расширению файла
    // stbi_load возвращает указатель на данные или nullptr при ошибке
    // Последний параметр - желаемое количество каналов (0 = как в файле)
    const auto desired_channels = 3; // Принудительно загружаем как RGB
    data_ = stbi_load(filename.c_str(), &width_, &height_, &channels_,
                      desired_channels);

    if (data_ == nullptr)
    {
        std::cerr << "Ошибка загрузки изображения: " << filename << std::endl;
        std::cerr << "Причина: " << stbi_failure_reason() << std::endl;
        return false;
    }

    // Принудительно устанавливаем 3 канала для RGB
    channels_ = 3;

    return true;
}

bool ImageProcessor::saveToFile(const std::string& filename) const
{
    if (!isValid())
    {
        std::cerr << "Ошибка: изображение не загружено" << std::endl;
        return false;
    }

    // Определяем формат по расширению файла
    auto extension = filename.substr(filename.find_last_of('.') + 1);

    // Преобразуем расширение в нижний регистр для сравнения
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    auto result = 0;

    if (extension == "jpg" || extension == "jpeg")
    {
        // Сохраняем как JPEG с качеством 90 (0-100, где 100 - наилучшее качество)
        result = stbi_write_jpg(filename.c_str(), width_, height_, channels_,
                                data_, 90);
    }
    else if (extension == "png")
    {
        // Сохраняем как PNG
        // stride_in_bytes = 0 означает автоматический расчет шага (width * channels)
        result = stbi_write_png(filename.c_str(), width_, height_, channels_,
                                data_, 0);
    }
    else
    {
        std::cerr << "Неподдерживаемый формат файла: " << extension << std::endl;
        return false;
    }

    if (result == 0)
    {
        std::cerr << "Ошибка сохранения изображения: " << filename << std::endl;
        return false;
    }

    return true;
}

int ImageProcessor::getWidth() const noexcept { return width_; }
int ImageProcessor::getHeight() const noexcept { return height_; }
int ImageProcessor::getChannels() const noexcept { return channels_; }
uint8_t* ImageProcessor::getData() noexcept { return data_; }
const uint8_t* ImageProcessor::getData() const noexcept { return data_; }
bool ImageProcessor::isValid() const noexcept { return data_ != nullptr; }

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

    const auto new_size = static_cast<size_t>(new_width) * new_height * channels_;

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
