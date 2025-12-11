#pragma once

#include <filters/IFilter.h>

/**
 * @brief Фильтр преобразования изображения в оттенки серого
 * 
 * Реализует стандартную формулу преобразования RGB в градации серого:
 * Y = 0.299*R + 0.587*G + 0.114*B
 * 
 * Эти коэффициенты основаны на восприятии яркости человеческим глазом.
 */
class GrayscaleFilter : public IFilter {
public:
    /**
     * @brief Применяет фильтр к изображению
     * @param image Обрабатываемое изображение
     * @return FilterResult с кодом ошибки
     */
    FilterResult apply(ImageProcessor& image) override;

    std::string getName() const override;
    std::string getDescription() const override;
    std::string getCategory() const override;
};

