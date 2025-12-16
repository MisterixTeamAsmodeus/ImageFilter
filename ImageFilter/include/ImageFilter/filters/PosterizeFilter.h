#pragma once

#include <filters/IFilter.h>

/**
 * @brief Фильтр постеризации
 * 
 * Уменьшает количество цветовых уровней в изображении, создавая
 * эффект плаката или комикса. Каждый канал квантуется на заданное
 * количество уровней.
 */
class PosterizeFilter : public IFilter {
public:
    /**
     * @brief Конструктор фильтра постеризации
     * @param levels Количество уровней для каждого канала (2-256, по умолчанию 4)
     *                При некорректном значении используется 4
     */
    explicit PosterizeFilter(int levels = 4) 
        : levels_((levels >= 2 && levels <= 256) ? levels : 4) {}

    /**
     * @brief Применяет фильтр постеризации к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    FilterResult apply(ImageProcessor& image) override;

    std::string getName() const override;
    std::string getDescription() const override;
    std::string getCategory() const override;

private:
    int levels_;  // Количество уровней
};



