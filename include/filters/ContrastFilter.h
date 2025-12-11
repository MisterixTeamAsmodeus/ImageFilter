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
     *               Должен быть > 0. При некорректном значении используется 1.5
     */
    explicit ContrastFilter(double factor = 1.5) : factor_(factor > 0.0 ? factor : 1.5) {}

    /**
     * @brief Применяет фильтр контрастности к изображению
     * @param image Обрабатываемое изображение
     * @return FilterResult с кодом ошибки
     */
    FilterResult apply(ImageProcessor& image) override;

    std::string getName() const override;
    std::string getDescription() const override;
    std::string getCategory() const override;

private:
    double factor_;  // Коэффициент контрастности
};



