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
     *                 При некорректном значении используется 0.5
     */
    explicit VignetteFilter(double strength = 0.5) 
        : strength_((strength >= 0.0 && strength <= 1.0) ? strength : 0.5) {}

    /**
     * @brief Применяет фильтр виньетирования к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    FilterResult apply(ImageProcessor& image) override;

    std::string getName() const override;
    std::string getDescription() const override;
    std::string getCategory() const override;

private:
    double strength_;  // Сила эффекта виньетирования
};



