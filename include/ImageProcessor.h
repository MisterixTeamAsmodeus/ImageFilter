#pragma once

#include <cstdint>
#include <string>

/**
 * @brief Класс для работы с изображениями в формате JPEG
 *
 * Использует библиотеку STB Image для загрузки и сохранения изображений.
 * Хранит данные изображения в виде непрерывного массива пикселей в формате RGB.
 */
class ImageProcessor
{
public:
    /**
     * @brief Конструктор по умолчанию
     */
    ImageProcessor() = default;

    /**
     * @brief Деструктор - освобождает память изображения
     */
    ~ImageProcessor();

    /**
     * @brief Загружает изображение из файла
     * @param filename Путь к файлу изображения
     * @return true если загрузка успешна, false в противном случае
     */
    bool loadFromFile(const std::string& filename);

    /**
     * @brief Сохраняет изображение в файл
     * @param filename Путь к выходному файлу
     * @return true если сохранение успешно, false в противном случае
     */
    bool saveToFile(const std::string& filename) const;

    /**
     * @brief Получает ширину изображения
     * @return Ширина в пикселях
     */
    [[nodiscard]] int getWidth() const noexcept;

    /**
     * @brief Получает высоту изображения
     * @return Высота в пикселях
     */
    [[nodiscard]] int getHeight() const noexcept;

    /**
     * @brief Получает количество каналов (обычно 3 для RGB)
     * @return Количество каналов
     */
    [[nodiscard]] int getChannels() const noexcept;

    /**
     * @brief Получает указатель на данные изображения
     * @return Указатель на массив пикселей (формат RGB, размер width * height *
     * channels)
     */
    [[nodiscard]] uint8_t* getData() noexcept;

    /**
     * @brief Получает константный указатель на данные изображения
     * @return Константный указатель на массив пикселей
     */
    [[nodiscard]] const uint8_t* getData() const noexcept;

    /**
     * @brief Проверяет, загружено ли изображение
     * @return True если изображение загружено
     */
    [[nodiscard]] bool isValid() const noexcept;

    /**
     * @brief Изменяет размеры изображения и заменяет данные
     * @param new_width Новая ширина изображения
     * @param new_height Новая высота изображения
     * @param new_data Указатель на новые данные изображения (должен быть размером new_width * new_height * channels)
     * @return true если операция успешна, false в противном случае
     * 
     * Принимает владение над new_data. Старые данные освобождаются через stbi_image_free.
     * Если new_data == nullptr, то просто освобождается старое изображение и устанавливаются новые размеры.
     */
    bool resize(int new_width, int new_height, uint8_t* new_data = nullptr);

private:
    int width_ = 0; // Ширина изображения
    int height_ = 0; // Высота изображения
    int channels_ = 0; // Количество каналов (3 для RGB)

    uint8_t* data_; // Данные изображения (RGB формат)
};
