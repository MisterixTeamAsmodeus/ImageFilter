#pragma once

#include <utils/SafeMath.h>
#include <cstddef>

/**
 * @brief Утилиты для безопасного вычисления смещений пикселей
 * 
 * Предоставляет функции для безопасного вычисления смещений пикселей в изображении
 * с защитой от переполнения, устраняя дублирование кода в различных фильтрах.
 */
namespace PixelOffsetUtils
{
    /**
     * @brief Вычисляет смещение строки (row offset) с защитой от переполнения
     * 
     * Вычисляет смещение начала строки y в изображении:
     * row_offset = y * width * channels
     * 
     * @param y Номер строки
     * @param width Ширина изображения
     * @param channels Количество каналов
     * @param row_offset Результат вычисления (выходной параметр)
     * @return true если вычисление успешно, false если произошло переполнение
     */
    inline bool computeRowOffset(int y, int width, int channels, size_t& row_offset)
    {
        size_t y_width = 0;
        if (!SafeMath::safeMultiply(static_cast<size_t>(y), static_cast<size_t>(width), y_width))
        {
            return false;
        }
        return SafeMath::safeMultiply(y_width, static_cast<size_t>(channels), row_offset);
    }

    /**
     * @brief Вычисляет смещение пикселя (pixel offset) с защитой от переполнения
     * 
     * Вычисляет смещение пикселя (x, y) в изображении:
     * pixel_offset = row_offset + x * channels
     * 
     * @param row_offset Смещение строки (результат computeRowOffset)
     * @param x Координата X пикселя
     * @param channels Количество каналов
     * @param pixel_offset Результат вычисления (выходной параметр)
     * @return true если вычисление успешно, false если произошло переполнение
     */
    inline bool computePixelOffset(size_t row_offset, int x, int channels, size_t& pixel_offset)
    {
        size_t x_channels = 0;
        if (!SafeMath::safeMultiply(static_cast<size_t>(x), static_cast<size_t>(channels), x_channels))
        {
            return false;
        }
        return SafeMath::safeAdd(row_offset, x_channels, pixel_offset);
    }

    /**
     * @brief Вычисляет смещение пикселя напрямую из координат с защитой от переполнения
     * 
     * Вычисляет смещение пикселя (x, y) в изображении:
     * pixel_offset = (y * width + x) * channels
     * 
     * @param x Координата X пикселя
     * @param y Координата Y пикселя
     * @param width Ширина изображения
     * @param channels Количество каналов
     * @param pixel_offset Результат вычисления (выходной параметр)
     * @return true если вычисление успешно, false если произошло переполнение
     */
    inline bool computePixelOffsetDirect(int x, int y, int width, int channels, size_t& pixel_offset)
    {
        size_t row_offset = 0;
        if (!computeRowOffset(y, width, channels, row_offset))
        {
            return false;
        }
        return computePixelOffset(row_offset, x, channels, pixel_offset);
    }

    /**
     * @brief Вычисляет смещение канала пикселя с защитой от переполнения
     * 
     * Вычисляет смещение канала c пикселя (x, y):
     * channel_offset = pixel_offset + c
     * 
     * @param pixel_offset Смещение пикселя
     * @param channel Индекс канала
     * @param channel_offset Результат вычисления (выходной параметр)
     * @return true если вычисление успешно, false если произошло переполнение
     */
    inline bool computeChannelOffset(size_t pixel_offset, int channel, size_t& channel_offset)
    {
        return SafeMath::safeAdd(pixel_offset, static_cast<size_t>(channel), channel_offset);
    }
}

