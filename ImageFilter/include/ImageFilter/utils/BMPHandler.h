#pragma once

#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief Обработчик для работы с BMP форматом
 * 
 * Предоставляет функции для чтения и записи BMP файлов.
 * Поддерживает 24-битные RGB изображения (без сжатия).
 */
namespace BMPHandler
{
    /**
     * @brief Загружает BMP изображение из файла
     * 
     * @param filename Путь к BMP файлу
     * @param width Ширина изображения (выходной параметр)
     * @param height Высота изображения (выходной параметр)
     * @param channels Количество каналов (выходной параметр, всегда 3 для BMP)
     * @return Указатель на данные изображения (RGB) или nullptr при ошибке
     * 
     * @note Вызывающий код должен освободить память через std::free()
     */
    uint8_t* loadBMP(const std::string& filename, int& width, int& height, int& channels);

    /**
     * @brief Сохраняет изображение в BMP файл
     * 
     * @param filename Путь к выходному BMP файлу
     * @param width Ширина изображения
     * @param height Высота изображения
     * @param channels Количество каналов (3 для RGB, 4 для RGBA - будет преобразовано в RGB)
     * @param data Данные изображения (RGB или RGBA)
     * @return true если сохранение успешно, false в противном случае
     */
    bool saveBMP(const std::string& filename, int width, int height, int channels, const uint8_t* data);
}

