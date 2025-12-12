#pragma once

#include <filters/IFilter.h>

/**
 * @brief Фильтр добавления шума
 * 
 * Добавляет случайный шум к изображению. Может использоваться для
 * тестирования алгоритмов удаления шума или создания художественных эффектов.
 */
class NoiseFilter : public IFilter {
public:
    /**
     * @brief Конструктор фильтра шума
     * @param intensity Интенсивность шума (0.0 - 1.0, где 1.0 = максимальный шум, по умолчанию 0.1)
     */
    explicit NoiseFilter(double intensity = 0.1) : intensity_(intensity) {}

    /**
     * @brief Применяет фильтр шума к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    bool apply(ImageProcessor& image) override;

private:
    double intensity_;  // Интенсивность шума
};

