#pragma once

#include <filters/IFilter.h>

/**
 * @brief Фильтр инверсии цветов
 * 
 * Инвертирует все цвета изображения, создавая негативный эффект.
 * Каждый канал преобразуется по формуле: new_value = 255 - old_value
 */
class InvertFilter : public IFilter {
public:
    /**
     * @brief Применяет фильтр инверсии к изображению
     * @param image Обрабатываемое изображение
     * @return FilterResult с кодом ошибки
     */
    FilterResult apply(ImageProcessor& image) override;

    std::string getName() const override;
    std::string getDescription() const override;
    std::string getCategory() const override;
};



