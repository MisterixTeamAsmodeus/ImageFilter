#pragma once

#include <cstdint>
#include <string>
#include <utils/FilterResult.h>

/**
 * @brief Класс для сохранения изображений в файлы
 * 
 * Отвечает за:
 * - Сохранение изображений в различных форматах (JPEG, PNG, BMP)
 * - Валидацию путей и данных изображения
 * - Преобразование форматов при сохранении (RGBA -> RGB для JPEG)
 * - Обработку ошибок сохранения
 */
class ImageSaver
{
public:
    /**
     * @brief Сохраняет изображение в файл
     * @param filename Путь к выходному файлу
     * @param data Указатель на данные изображения
     * @param width Ширина изображения
     * @param height Высота изображения
     * @param channels Количество каналов (3 для RGB, 4 для RGBA)
     * @param preserve_alpha Если true, сохраняет альфа-канал (для PNG), если false - принудительно RGB
     * @param jpeg_quality Качество сохранения JPEG (0-100)
     * @return FilterResult с результатом операции
     * 
     * @note Путь к файлу валидируется на безопасность (защита от path traversal атак).
     */
    static FilterResult saveToFile(const std::string& filename,
                                    const uint8_t* data,
                                    int width,
                                    int height,
                                    int channels,
                                    bool preserve_alpha,
                                    int jpeg_quality);
};

