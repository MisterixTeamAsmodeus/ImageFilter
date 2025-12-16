#pragma once

#include <filters/IFilter.h>
#include <utils/BorderHandler.h>

// Forward declaration
class IBufferPool;

/**
 * @brief Фильтр повышения резкости изображения
 * 
 * Реализует алгоритм повышения резкости с использованием unsharp masking.
 * Применяет ядро повышения резкости, которое усиливает края и детали изображения.
 * 
 * Базовое ядро (3x3) при strength = 1.0:
 *   0  -1   0
 *  -1   5  -1
 *   0  -1   0
 * 
 * Параметр strength позволяет регулировать силу эффекта:
 * - strength = 0.0: изображение не изменяется
 * - strength = 1.0: стандартное повышение резкости
 * - strength > 1.0: более сильное повышение резкости
 * 
 * Ядро вычисляется динамически на основе параметра strength для более гибкого контроля.
 */
class SharpenFilter : public IFilter {
public:
    /**
     * @brief Конструктор фильтра повышения резкости
     * @param strength Сила эффекта резкости (по умолчанию 1.0, где 1.0 = стандартная резкость)
     *                 Должен быть >= 0. При некорректном значении используется 1.0
     * @param borderStrategy Стратегия обработки границ (по умолчанию Mirror)
     * @param buffer_pool Пул буферов для переиспользования временных буферов (опционально)
     */
    explicit SharpenFilter(double strength = 1.0,
                           BorderHandler::Strategy borderStrategy = BorderHandler::Strategy::Mirror,
                           IBufferPool* buffer_pool = nullptr) 
        : strength_(strength >= 0.0 ? strength : 1.0), 
          border_handler_(borderStrategy), 
          buffer_pool_(buffer_pool) {}

    /**
     * @brief Применяет фильтр повышения резкости
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    FilterResult apply(ImageProcessor& image) override;

    std::string getName() const override;
    std::string getDescription() const override;
    std::string getCategory() const override;

private:
    double strength_;  // Сила эффекта резкости
    BorderHandler border_handler_;  // Обработчик границ
    IBufferPool* buffer_pool_;  // Пул буферов для переиспользования (может быть nullptr)
};

