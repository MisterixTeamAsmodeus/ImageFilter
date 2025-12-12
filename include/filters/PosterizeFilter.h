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
     */
    explicit PosterizeFilter(int levels = 4) : levels_(levels) {}

    /**
     * @brief Применяет фильтр постеризации к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    bool apply(ImageProcessor& image) override;

private:
    int levels_;  // Количество уровней
};



