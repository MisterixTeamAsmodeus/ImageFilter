#pragma once

#include <filters/IFilter.h>

/**
 * @brief Фильтр горизонтального отражения изображения
 * 
 * Отражает изображение по вертикальной оси (зеркалирует слева направо).
 */
class FlipHorizontalFilter : public IFilter {
public:
    /**
     * @brief Применяет фильтр горизонтального отражения к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    bool apply(ImageProcessor& image) override;
};



