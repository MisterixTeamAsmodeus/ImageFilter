#pragma once

#include <filters/IFilter.h>

/**
 * @brief Фильтр пороговой бинаризации
 * 
 * Преобразует изображение в черно-белое на основе порогового значения.
 * Пиксели выше порога становятся белыми, ниже порога - черными.
 */
class ThresholdFilter : public IFilter {
public:
    /**
     * @brief Конструктор фильтра пороговой бинаризации
     * @param threshold Пороговое значение (0-255, по умолчанию 128)
     */
    explicit ThresholdFilter(int threshold = 128) : threshold_(threshold) {}

    /**
     * @brief Применяет фильтр пороговой бинаризации к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    bool apply(ImageProcessor& image) override;

private:
    int threshold_;  // Пороговое значение
};



