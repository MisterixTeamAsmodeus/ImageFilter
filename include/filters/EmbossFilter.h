#pragma once

#include <filters/IFilter.h>
#include <utils/BorderHandler.h>

/**
 * @brief Фильтр рельефа (emboss)
 * 
 * Создает эффект рельефа, придавая изображению трехмерный вид.
 * Использует ядро свертки для выделения краёв и создания эффекта глубины.
 */
class EmbossFilter : public IFilter {
public:
    /**
     * @brief Конструктор фильтра рельефа
     * @param borderStrategy Стратегия обработки границ (по умолчанию Mirror)
     */
    explicit EmbossFilter(BorderHandler::Strategy borderStrategy = BorderHandler::Strategy::Mirror) 
        : border_handler_(borderStrategy) {}

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
    BorderHandler border_handler_;  // Обработчик границ
};



