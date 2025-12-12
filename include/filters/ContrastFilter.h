#pragma once

#include <filters/IFilter.h>

/**
 * @brief Фильтр изменения контрастности изображения
 * 
 * Изменяет контрастность изображения. Увеличивает разницу между светлыми
 * и темными областями.
 * 
 * Формула: new_value = ((old_value - 128) * factor) + 128
 * где factor > 1.0 увеличивает контраст, factor < 1.0 уменьшает контраст
 */
class ContrastFilter : public IFilter {
public:
    /**
     * @brief Конструктор фильтра контрастности
     * @param factor Коэффициент контрастности (1.0 = без изменений, >1.0 = больше контраст, <1.0 = меньше контраст)
     */
    explicit ContrastFilter(double factor = 1.5) : factor_(factor) {}

    /**
     * @brief Применяет фильтр контрастности к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    bool apply(ImageProcessor& image) override;

private:
    double factor_;  // Коэффициент контрастности
};



