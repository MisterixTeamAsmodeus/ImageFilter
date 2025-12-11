#pragma once

#include <filters/IFilter.h>
#include <utils/BorderHandler.h>

/**
 * @brief Фильтр повышения резкости изображения
 * 
 * Реализует алгоритм повышения резкости с использованием unsharp masking.
 * Применяет ядро повышения резкости, которое усиливает края и детали изображения.
 * 
 * Используемое ядро (3x3):
 *   0  -1   0
 *  -1   5  -1
 *   0  -1   0
 * 
 * Это классическое ядро для повышения резкости, которое вычитает размытую версию
 * из оригинала, усиливая тем самым края.
 */
class SharpenFilter : public IFilter {
public:
    /**
     * @brief Конструктор фильтра повышения резкости
     * @param borderStrategy Стратегия обработки границ (по умолчанию Mirror)
     */
    explicit SharpenFilter(BorderHandler::Strategy borderStrategy = BorderHandler::Strategy::Mirror) 
        : border_handler_(borderStrategy) {}

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
    BorderHandler border_handler_;  // Обработчик границ
};

