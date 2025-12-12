#pragma once

#include <filters/IFilter.h>

/**
 * @brief Медианный фильтр
 * 
 * Удаляет шум из изображения, заменяя каждый пиксель медианой
 * значений в окрестности. Эффективен для удаления солевого и перцового шума.
 */
class MedianFilter : public IFilter {
public:
    /**
     * @brief Конструктор медианного фильтра
     * @param radius Радиус окна (размер окна = 2*radius + 1, по умолчанию 2)
     */
    explicit MedianFilter(int radius = 2) : radius_(radius) {}

    /**
     * @brief Применяет медианный фильтр к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    bool apply(ImageProcessor& image) override;

private:
    int radius_;  // Радиус окна
};



