#pragma once

#include <filters/IFilter.h>

/**
 * @brief Фильтр виньетирования
 * 
 * Создает эффект затемнения по краям изображения, имитируя
 * эффект старых фотографий или объективов камеры.
 */
class VignetteFilter : public IFilter {
public:
    /**
     * @brief Конструктор фильтра виньетирования
     * @param strength Сила эффекта (0.0 - 1.0, где 1.0 = максимальное затемнение, по умолчанию 0.5)
     */
    explicit VignetteFilter(double strength = 0.5) : strength_(strength) {}

    /**
     * @brief Применяет фильтр виньетирования к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    bool apply(ImageProcessor& image) override;

private:
    double strength_;  // Сила эффекта виньетирования
};



