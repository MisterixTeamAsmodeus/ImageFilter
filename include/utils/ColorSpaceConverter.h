#pragma once

#include <utils/FilterResult.h>
#include <cstdint>

/**
 * @brief Класс для преобразования между различными цветовыми пространствами
 * 
 * Предоставляет функции для преобразования между RGB и RGBA форматами.
 */
class ColorSpaceConverter
{
public:
    /**
     * @brief Конструктор по умолчанию
     */
    ColorSpaceConverter() = default;

    /**
     * @brief Деструктор
     */
    ~ColorSpaceConverter() = default;

    // Запрещаем копирование
    ColorSpaceConverter(const ColorSpaceConverter&) = delete;
    ColorSpaceConverter& operator=(const ColorSpaceConverter&) = delete;

    /**
     * @brief Преобразует RGBA данные в RGB данные
     * 
     * @param rgba_data Исходные RGBA данные
     * @param width Ширина изображения
     * @param height Высота изображения
     * @param rgb_data Выходной буфер для RGB данных (должен быть выделен)
     * @return FilterResult с результатом операции
     */
    static FilterResult convertRGBAToRGB(const uint8_t* rgba_data, 
                                         int width, 
                                         int height, 
                                         uint8_t* rgb_data);

    /**
     * @brief Преобразует RGB данные в RGBA данные
     * 
     * @param rgb_data Исходные RGB данные
     * @param width Ширина изображения
     * @param height Высота изображения
     * @param rgba_data Выходной буфер для RGBA данных (должен быть выделен)
     * @return FilterResult с результатом операции
     */
    static FilterResult convertRGBToRGBA(const uint8_t* rgb_data, 
                                        int width, 
                                        int height, 
                                        uint8_t* rgba_data);
};

