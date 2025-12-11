#pragma once

#include <filters/IFilter.h>

/**
 * @brief Фильтр вертикального отражения изображения
 * 
 * Отражает изображение по горизонтальной оси (зеркалирует сверху вниз).
 */
class FlipVerticalFilter : public IFilter {
public:
    /**
     * @brief Применяет фильтр вертикального отражения к изображению
     * @param image Обрабатываемое изображение
     * @return FilterResult с кодом ошибки
     */
    FilterResult apply(ImageProcessor& image) override;

    std::string getName() const override;
    std::string getDescription() const override;
    std::string getCategory() const override;
};



