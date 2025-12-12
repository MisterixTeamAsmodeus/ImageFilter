#pragma once

#include <filters/IFilter.h>

/**
 * @brief Фильтр вертикального отражения изображения
 * 
 * Отражает изображение по горизонтальной оси (зеркалирует сверху вниз).
 */
class FlipVerticalFilter : public IFilter {
public:
    /**
     * @brief Применяет фильтр вертикального отражения к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    bool apply(ImageProcessor& image) override;
};



