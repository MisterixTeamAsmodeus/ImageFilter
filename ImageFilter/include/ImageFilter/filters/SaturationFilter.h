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
     *               Должен быть > 0. При некорректном значении используется 1.5
     */
    explicit SaturationFilter(double factor = 1.5) : factor_(factor > 0.0 ? factor : 1.5) {}

    /**
     * @brief Применяет фильтр насыщенности к изображению
     * @param image Обрабатываемое изображение
     * @return FilterResult с кодом ошибки
     */
    FilterResult apply(ImageProcessor& image) override;

    std::string getName() const override;
    std::string getDescription() const override;
    std::string getCategory() const override;

private:
    double factor_;  // Коэффициент насыщенности
};



