#pragma once

#include <filters/IFilter.h>

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
     * @brief Применяет фильтр детекции краёв к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    bool apply(ImageProcessor& image) override;
};



