#pragma once

#include <cstdint>
#include <string>

/**
 * @brief Класс для работы с изображениями в форматах JPEG и PNG
 *
 * Использует библиотеку STB Image для загрузки и сохранения изображений.
 * Хранит данные изображения в виде непрерывного массива пикселей в формате RGB или RGBA.
 * Поддерживает как 3 канала (RGB), так и 4 канала (RGBA) для работы с альфа-каналом.
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

    // Запрещаем копирование (данные владеют памятью через stbi_image_free)
    ImageProcessor(const ImageProcessor&) = delete;
    ImageProcessor& operator=(const ImageProcessor&) = delete;

    /**
     * @brief Move-конструктор
     * @param other Другой объект ImageProcessor, из которого переносятся данные
     */
    ImageProcessor(ImageProcessor&& other) noexcept;

    /**
     * @brief Move-оператор присваивания
     * @param other Другой объект ImageProcessor, из которого переносятся данные
     * @return Ссылка на текущий объект
     */
    ImageProcessor& operator=(ImageProcessor&& other) noexcept;

    /**
     * @brief Загружает изображение из файла
     * @param filename Путь к файлу изображения
     * @param preserve_alpha Если true, загружает с альфа-каналом (RGBA), если false - принудительно RGB
     * @return true если загрузка успешна, false в противном случае
     * 
     * @note Путь к файлу валидируется на безопасность (защита от path traversal атак).
     * Размер файла ограничен DEFAULT_MAX_IMAGE_SIZE (1 GB по умолчанию).
     */
    bool loadFromFile(const std::string& filename, bool preserve_alpha = false);

    /**
     * @brief Сохраняет изображение в файл
     * @param filename Путь к выходному файлу
     * @param preserve_alpha Если true, сохраняет альфа-канал (для PNG), если false - принудительно RGB
     * @return true если сохранение успешно, false в противном случае
     * 
     * @note Путь к файлу валидируется на безопасность (защита от path traversal атак).
     */
    bool saveToFile(const std::string& filename, bool preserve_alpha = false) const;

    /**
     * @brief Преобразует RGBA изображение в RGB, удаляя альфа-канал
     * @return true если преобразование успешно, false если изображение уже RGB или невалидно
     */
    bool convertToRGB();

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
     * @brief Получает количество каналов (3 для RGB или 4 для RGBA)
     * @return Количество каналов
     */
    [[nodiscard]] int getChannels() const noexcept;

    /**
     * @brief Проверяет, имеет ли изображение альфа-канал
     * @return true если изображение имеет 4 канала (RGBA)
     */
    [[nodiscard]] bool hasAlpha() const noexcept;

    /**
     * @brief Получает указатель на данные изображения
     * @return Указатель на массив пикселей (формат RGB или RGBA, размер width * height * channels)
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
     * @brief Устанавливает качество сохранения JPEG изображений
     * @param quality Качество в диапазоне 0-100 (0 - худшее качество, 100 - лучшее качество)
     * @return true если качество установлено успешно, false если значение вне допустимого диапазона
     * 
     * Значение по умолчанию: 90
     * Применяется только при сохранении в формате JPEG, для PNG не влияет на результат
     */
    bool setJpegQuality(int quality);

    /**
     * @brief Получает текущее качество сохранения JPEG изображений
     * @return Качество в диапазоне 0-100
     */
    [[nodiscard]] int getJpegQuality() const noexcept;

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

    /**
     * @brief Изменяет размеры изображения и заменяет данные с указанием количества каналов
     * @param new_width Новая ширина изображения
     * @param new_height Новая высота изображения
     * @param new_channels Количество каналов (3 для RGB, 4 для RGBA)
     * @param new_data Указатель на новые данные изображения (должен быть размером new_width * new_height * new_channels)
     * @return true если операция успешна, false в противном случае
     * 
     * Принимает владение над new_data. Старые данные освобождаются через stbi_image_free.
     * Если new_data == nullptr, то просто освобождается старое изображение и устанавливаются новые размеры и каналы.
     */
    bool resize(int new_width, int new_height, int new_channels, uint8_t* new_data);

private:
    /**
     * @note Поля упорядочены для минимизации padding: сначала указатель (требует выравнивания 8),
     * затем int поля (выравнивание 4) для оптимального использования памяти.
     */
    uint8_t* data_ = nullptr; // Данные изображения (RGB или RGBA формат)
    int width_ = 0; // Ширина изображения
    int height_ = 0; // Высота изображения
    int channels_ = 0; // Количество каналов (3 для RGB или 4 для RGBA)
    int jpeg_quality_ = 90; // Качество сохранения JPEG (0-100, по умолчанию 90)
};
