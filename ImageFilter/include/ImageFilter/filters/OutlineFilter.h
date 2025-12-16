#pragma once

#include <filters/IFilter.h>
#include <utils/BorderHandler.h>

/**
 * @brief Фильтр контуров
 * 
 * Выделяет контуры объектов в изображении, используя лапласиан.
 * Результат - черно-белое изображение с четкими контурами.
 */
class OutlineFilter : public IFilter {
public:
    /**
     * @brief Конструктор фильтра контуров
     * @param borderStrategy Стратегия обработки границ (по умолчанию Mirror)
     */
    explicit OutlineFilter(BorderHandler::Strategy borderStrategy = BorderHandler::Strategy::Mirror) 
        : border_handler_(borderStrategy) {}

    /**
     * @brief Применяет фильтр контуров к изображению
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



