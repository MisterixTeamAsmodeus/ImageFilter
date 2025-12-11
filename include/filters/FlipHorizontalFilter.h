#pragma once

#include <filters/IFilter.h>

/**
 * @brief Фильтр горизонтального отражения изображения
 * 
 * Отражает изображение по вертикальной оси (зеркалирует слева направо).
 */
class FlipHorizontalFilter : public IFilter {
public:
    /**
     * @brief Применяет фильтр горизонтального отражения к изображению
     * @param image Обрабатываемое изображение
     * @return FilterResult с кодом ошибки
     */
    FilterResult apply(ImageProcessor& image) override;

    std::string getName() const override;
    std::string getDescription() const override;
    std::string getCategory() const override;
};



