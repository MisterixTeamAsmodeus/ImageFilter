#pragma once

#include <filters/IFilter.h>

/**
 * @brief Фильтр изменения насыщенности цветов
 * 
 * Изменяет насыщенность цветов изображения, не затрагивая яркость.
 * 
 * Алгоритм:
 * 1. Преобразуем RGB в градации серого
 * 2. Интерполируем между серым и оригинальным цветом: new = gray + (original - gray) * factor
 */
class SaturationFilter : public IFilter {
public:
    /**
     * @brief Конструктор фильтра насыщенности
     * @param factor Коэффициент насыщенности (1.0 = без изменений, >1.0 = более насыщенные цвета, <1.0 = менее насыщенные, 0.0 = градации серого)
     */
    explicit SaturationFilter(double factor = 1.5) : factor_(factor) {}

    /**
     * @brief Применяет фильтр насыщенности к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    bool apply(ImageProcessor& image) override;

private:
    double factor_;  // Коэффициент насыщенности
};



