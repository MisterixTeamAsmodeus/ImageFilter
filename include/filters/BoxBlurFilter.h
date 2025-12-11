#pragma once

#include <filters/IFilter.h>
#include <utils/BorderHandler.h>

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
     *               Должен быть >= 0. При некорректном значении используется 5
     * @param borderStrategy Стратегия обработки границ (по умолчанию Mirror)
     */
    explicit BoxBlurFilter(int radius = 5, 
                           BorderHandler::Strategy borderStrategy = BorderHandler::Strategy::Mirror) 
        : radius_(radius >= 0 ? radius : 5), border_handler_(borderStrategy) {}

    /**
     * @brief Применяет фильтр размытия по прямоугольнику к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    FilterResult apply(ImageProcessor& image) override;

    std::string getName() const override;
    std::string getDescription() const override;
    std::string getCategory() const override;

private:
    int radius_;  // Радиус размытия
    BorderHandler border_handler_;  // Обработчик границ
};



