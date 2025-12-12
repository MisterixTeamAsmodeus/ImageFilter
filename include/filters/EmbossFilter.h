#pragma once

#include <filters/IFilter.h>

/**
 * @brief Фильтр рельефа (emboss)
 * 
 * Создает эффект рельефа, придавая изображению трехмерный вид.
 * Использует ядро свертки для выделения краёв и создания эффекта глубины.
 */
class EmbossFilter : public IFilter {
public:
    /**
     * @brief Применяет фильтр рельефа к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    bool apply(ImageProcessor& image) override;
};



