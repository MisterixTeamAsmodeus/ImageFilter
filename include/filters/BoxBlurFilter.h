#pragma once

#include <filters/IFilter.h>

/**
 * @brief Фильтр размытия по прямоугольнику (Box Blur)
 * 
 * Простой и быстрый алгоритм размытия, использующий равномерное распределение
 * весов в окне. Быстрее, чем Gaussian Blur, но дает менее качественный результат.
 * Использует separable kernel для оптимизации.
 */
class BoxBlurFilter : public IFilter {
public:
    /**
     * @brief Конструктор фильтра размытия по прямоугольнику
     * @param radius Радиус размытия (размер окна = 2*radius + 1, по умолчанию 5)
     */
    explicit BoxBlurFilter(int radius = 5) : radius_(radius) {}

    /**
     * @brief Применяет фильтр размытия по прямоугольнику к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    bool apply(ImageProcessor& image) override;

private:
    int radius_;  // Радиус размытия
};



