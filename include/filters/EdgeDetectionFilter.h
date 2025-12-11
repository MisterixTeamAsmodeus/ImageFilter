#pragma once

#include <filters/IFilter.h>
#include <utils/BorderHandler.h>

/**
 * @brief Фильтр детекции краёв (оператор Собеля)
 * 
 * Обнаруживает края в изображении, используя оператор Собеля.
 * Результат - изображение в градациях серого, где белые области
 * соответствуют краям, а черные - однородным областям.
 */
class EdgeDetectionFilter : public IFilter {
public:
    /**
     * @brief Конструктор фильтра детекции краёв
     * @param borderStrategy Стратегия обработки границ (по умолчанию Mirror)
     */
    explicit EdgeDetectionFilter(BorderHandler::Strategy borderStrategy = BorderHandler::Strategy::Mirror) 
        : border_handler_(borderStrategy) {}

    /**
     * @brief Применяет фильтр детекции краёв к изображению
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



