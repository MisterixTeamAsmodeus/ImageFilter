#pragma once

#include <filters/IFilter.h>
#include <utils/BorderHandler.h>

/**
 * @brief Фильтр детекции краёв
 * 
 * Обнаруживает края в изображении, используя различные операторы градиента.
 * Результат - изображение в градациях серого, где белые области
 * соответствуют краям, а черные - однородным областям.
 * 
 * Поддерживает несколько операторов:
 * - Sobel: классический оператор, хороший баланс между точностью и производительностью
 * - Prewitt: более простой оператор, быстрее чем Sobel
 * - Scharr: более точный оператор, лучше определяет края под углом
 */
class EdgeDetectionFilter : public IFilter {
public:
    /**
     * @brief Тип оператора для детекции краёв
     */
    enum class Operator {
        Sobel,   ///< Оператор Собеля (по умолчанию)
        Prewitt,  ///< Оператор Преввитта
        Scharr    ///< Оператор Шарра
    };

    /**
     * @brief Конструктор фильтра детекции краёв
     * @param sensitivity Чувствительность детекции (0.0 - 1.0, где 1.0 = максимальная чувствительность)
     *                    Влияет на порог нормализации. По умолчанию 0.5
     * @param operator_type Тип оператора для детекции краёв (по умолчанию Sobel)
     * @param borderStrategy Стратегия обработки границ (по умолчанию Mirror)
     */
    explicit EdgeDetectionFilter(double sensitivity = 0.5,
                                 Operator operator_type = Operator::Sobel,
                                 BorderHandler::Strategy borderStrategy = BorderHandler::Strategy::Mirror) 
        : sensitivity_(sensitivity >= 0.0 && sensitivity <= 1.0 ? sensitivity : 0.5),
          operator_type_(operator_type),
          border_handler_(borderStrategy) {}

    /**
     * @brief Применяет фильтр детекции краёв к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    FilterResult apply(ImageProcessor& image) override;

    std::string getName() const override;
    std::string getDescription() const override;
    std::string getCategory() const override;

private:
    double sensitivity_;  // Чувствительность детекции краёв
    Operator operator_type_;  // Тип оператора для детекции краёв
    BorderHandler border_handler_;  // Обработчик границ
};



