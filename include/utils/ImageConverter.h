#pragma once

#include <cstdint>
#include <utils/FilterResult.h>

/**
 * @brief Класс для преобразования форматов изображений
 * 
 * Отвечает за:
 * - Преобразование RGBA в RGB
 * - Преобразование RGB в RGBA
 * - Композицию с фоном при преобразовании
 */
class ImageConverter
{
public:
    /**
     * @brief Преобразует RGBA изображение в RGB, удаляя альфа-канал
     * @param rgba_data Исходные RGBA данные
     * @param width Ширина изображения
     * @param height Высота изображения
     * @param rgb_data Выходной буфер для RGB данных (должен быть выделен)
     * @return FilterResult с результатом операции
     * 
     * Использует альфа-канал для композиции с белым фоном (alpha blending).
     */
    static FilterResult convertRGBAToRGB(const uint8_t* rgba_data,
                                         int width,
                                         int height,
                                         uint8_t* rgb_data);
};

