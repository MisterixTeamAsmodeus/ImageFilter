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
     *               Должен быть > 0. При некорректном значении используется 1.2
     */
    explicit BrightnessFilter(double factor = 1.2) : factor_(factor > 0.0 ? factor : 1.2) {}

    /**
     * @brief Применяет фильтр яркости к изображению
     * @param image Обрабатываемое изображение
     * @return FilterResult с кодом ошибки
     */
    FilterResult apply(ImageProcessor& image) override;

    std::string getName() const override;
    std::string getDescription() const override;
    std::string getCategory() const override;
    bool supportsInPlace() const noexcept override;

private:
    double factor_;  // Коэффициент яркости
};



