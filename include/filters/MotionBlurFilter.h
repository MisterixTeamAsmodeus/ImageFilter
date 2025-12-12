#pragma once

#include <filters/IFilter.h>

/**
 * @brief Фильтр размытия движения
 * 
 * Создает эффект размытия движения в заданном направлении.
 * Имитирует эффект съемки движущегося объекта.
 */
class MotionBlurFilter : public IFilter {
public:
    /**
     * @brief Конструктор фильтра размытия движения
     * @param length Длина размытия в пикселях (по умолчанию 10)
     * @param angle Угол направления размытия в градусах (0 = горизонтально, 90 = вертикально, по умолчанию 0)
     */
    explicit MotionBlurFilter(int length = 10, double angle = 0.0) : length_(length), angle_(angle) {}

    /**
     * @brief Применяет фильтр размытия движения к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    bool apply(ImageProcessor& image) override;

private:
    int length_;     // Длина размытия
    double angle_;   // Угол направления размытия в градусах
};



