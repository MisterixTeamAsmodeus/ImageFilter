#pragma once

#include <filters/IFilter.h>
#include <utils/BorderHandler.h>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <shared_mutex>

/**
 * @brief Фильтр размытия по Гауссу
 * 
 * Реализует алгоритм размытия с использованием separable kernel для оптимизации.
 * Вместо применения двумерного ядра размера NxN (O(N²) операций на пиксель),
 * применяется одномерное ядро дважды: сначала по горизонтали, затем по вертикали
 * (O(2N) операций на пиксель). Это дает значительный прирост производительности.
 * 
 * Особенности реализации:
 * - Использует separable kernel для оптимизации
 * - Поддерживает настраиваемый радиус размытия
 * - Использует std::vector для хранения промежуточных результатов
 */
class GaussianBlurFilter : public IFilter {
public:
    /**
     * @brief Конструктор фильтра размытия
     * @param radius Радиус размытия (по умолчанию 5.0)
     *               Должен быть > 0. При некорректном значении используется 5.0
     * @param borderStrategy Стратегия обработки границ (по умолчанию Mirror)
     */
    explicit GaussianBlurFilter(double radius = 5.0, 
                               BorderHandler::Strategy borderStrategy = BorderHandler::Strategy::Mirror) 
        : radius_(radius > 0.0 ? radius : 5.0), border_handler_(borderStrategy) {}

    /**
     * @brief Применяет фильтр размытия по Гауссу
     * @param image Обрабатываемое изображение
     * @return FilterResult с кодом ошибки
     */
    FilterResult apply(ImageProcessor& image) override;

    std::string getName() const override;
    std::string getDescription() const override;
    std::string getCategory() const override;

private:
    double radius_;  // Радиус размытия
    BorderHandler border_handler_;  // Обработчик границ
};

