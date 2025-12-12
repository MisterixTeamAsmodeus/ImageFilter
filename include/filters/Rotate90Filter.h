#pragma once

#include <filters/IFilter.h>

/**
 * @brief Фильтр поворота изображения на 90 градусов
 * 
 * Поворачивает изображение на 90 градусов по часовой стрелке или против.
 * При повороте размеры изображения меняются местами (width <-> height).
 */
class Rotate90Filter : public IFilter {
public:
    /**
     * @brief Конструктор фильтра поворота
     * @param clockwise true для поворота по часовой стрелке, false для поворота против часовой стрелки
     */
    explicit Rotate90Filter(bool clockwise = true) : clockwise_(clockwise) {}

    /**
     * @brief Применяет фильтр поворота к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    bool apply(ImageProcessor& image) override;

private:
    bool clockwise_;  // Направление поворота
};



