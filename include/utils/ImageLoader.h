#pragma once

#include <cstdint>
#include <string>
#include <utils/FilterResult.h>

/**
 * @brief Класс для загрузки изображений из файлов
 * 
 * Отвечает за:
 * - Загрузку изображений в различных форматах (JPEG, PNG, BMP)
 * - Валидацию путей и размеров файлов
 * - Преобразование форматов при загрузке
 * - Обработку ошибок загрузки
 */
class ImageLoader
{
public:
    /**
     * @brief Структура для хранения загруженных данных изображения
     */
    struct LoadedImage
    {
        uint8_t* data = nullptr;
        int width = 0;
        int height = 0;
        int channels = 0;
    };

    /**
     * @brief Загружает изображение из файла
     * @param filename Путь к файлу изображения
     * @param preserve_alpha Если true, загружает с альфа-каналом (RGBA), если false - принудительно RGB
     * @param result Структура для сохранения загруженных данных
     * @return FilterResult с результатом операции
     * 
     * @note Путь к файлу валидируется на безопасность (защита от path traversal атак).
     * Размер файла ограничен DEFAULT_MAX_IMAGE_SIZE (1 GB по умолчанию).
     */
    static FilterResult loadFromFile(const std::string& filename, 
                                     bool preserve_alpha, 
                                     LoadedImage& result);
};

