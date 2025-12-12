#pragma once

#include <filters/IFilter.h>

/**
 * @brief Фильтр контуров
 * 
 * Выделяет контуры объектов в изображении, используя лапласиан.
 * Результат - черно-белое изображение с четкими контурами.
 */
class OutlineFilter : public IFilter {
public:
    /**
     * @brief Применяет фильтр контуров к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    bool apply(ImageProcessor& image) override;
};



