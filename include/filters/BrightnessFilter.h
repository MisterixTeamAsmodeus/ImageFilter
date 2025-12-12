#pragma once

#include <filters/IFilter.h>

/**
 * @brief Фильтр изменения яркости изображения
 * 
 * Изменяет яркость всех пикселей изображения на заданный коэффициент.
 * 
 * Формула: new_value = old_value * factor
 * где factor > 1.0 увеличивает яркость, factor < 1.0 уменьшает яркость
 */
class BrightnessFilter : public IFilter {
public:
    /**
     * @brief Конструктор фильтра яркости
     * @param factor Коэффициент яркости (1.0 = без изменений, >1.0 = ярче, <1.0 = темнее)
     */
    explicit BrightnessFilter(double factor = 1.2) : factor_(factor) {}

    /**
     * @brief Применяет фильтр яркости к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    bool apply(ImageProcessor& image) override;

private:
    double factor_;  // Коэффициент яркости
};



