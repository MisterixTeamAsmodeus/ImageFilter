#pragma once

#include <filters/IFilter.h>
#include <utils/BorderHandler.h>

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
     *               Должен быть >= 0. При некорректном значении используется 2
     * @param borderStrategy Стратегия обработки границ (по умолчанию Mirror)
     */
    explicit MedianFilter(int radius = 2, 
                         BorderHandler::Strategy borderStrategy = BorderHandler::Strategy::Mirror) 
        : radius_(radius >= 0 ? radius : 2), border_handler_(borderStrategy) {}

    /**
     * @brief Применяет медианный фильтр к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    FilterResult apply(ImageProcessor& image) override;

    std::string getName() const override;
    std::string getDescription() const override;
    std::string getCategory() const override;

private:
    int radius_;  // Радиус окна
    BorderHandler border_handler_;  // Обработчик границ
};



