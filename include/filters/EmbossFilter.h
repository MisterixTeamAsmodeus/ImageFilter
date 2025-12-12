#pragma once

#include <filters/IFilter.h>
#include <utils/BorderHandler.h>

/**
 * @brief Фильтр рельефа (emboss)
 * 
 * Создает эффект рельефа, придавая изображению трехмерный вид.
 * Использует ядро свертки для выделения краёв и создания эффекта глубины.
 * 
 * Параметр strength позволяет регулировать силу эффекта:
 * - strength = 0.0: изображение не изменяется
 * - strength = 1.0: стандартный эффект рельефа
 * - strength > 1.0: более сильный эффект рельефа
 */
class EmbossFilter : public IFilter {
public:
    /**
     * @brief Конструктор фильтра рельефа
     * @param strength Сила эффекта рельефа (по умолчанию 1.0, где 1.0 = стандартный эффект)
     *                 Должен быть >= 0. При некорректном значении используется 1.0
     * @param borderStrategy Стратегия обработки границ (по умолчанию Mirror)
     */
    explicit EmbossFilter(double strength = 1.0,
                         BorderHandler::Strategy borderStrategy = BorderHandler::Strategy::Mirror) 
        : strength_(strength >= 0.0 ? strength : 1.0),
          border_handler_(borderStrategy) {}

    /**
     * @brief Применяет фильтр рельефа к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    FilterResult apply(ImageProcessor& image) override;

    std::string getName() const override;
    std::string getDescription() const override;
    std::string getCategory() const override;

private:
    double strength_;  // Сила эффекта рельефа
    BorderHandler border_handler_;  // Обработчик границ
};



