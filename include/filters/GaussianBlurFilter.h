#pragma once

#include <filters/IFilter.h>
#include <cstdint>
#include <vector>

/**
 * @brief Фильтр размытия по Гауссу
 * 
 * Реализует алгоритм размытия с использованием separable kernel для оптимизации.
 * Вместо применения двумерного ядра размера NxN (O(N²) операций на пиксель),
 * применяется одномерное ядро дважды: сначала по горизонтали, затем по вертикали
 * (O(2N) операций на пиксель). Это дает значительный прирост производительности.
 * 
 * Особенности реализации:
 * - Использует separable kernel для оптимизации
 * - Поддерживает настраиваемый радиус размытия
 * - Использует std::vector для хранения промежуточных результатов
 */
class GaussianBlurFilter : public IFilter {
public:
    /**
     * @brief Конструктор фильтра размытия
     * @param radius Радиус размытия (по умолчанию 5.0)
     */
    explicit GaussianBlurFilter(double radius = 5.0) : radius_(radius) {}

    /**
     * @brief Применяет фильтр размытия по Гауссу
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    bool apply(ImageProcessor& image) override;

private:
    /**
     * @brief Генерирует одномерное ядро Гаусса в целочисленном формате
     * @param radius Радиус размытия
     * @param sigma Стандартное отклонение (вычисляется из radius)
     * @return Вектор коэффициентов ядра (масштабированные на 65536 для точности)
     */
    static std::vector<int32_t> generateKernel(double radius, const double& sigma);

    /**
     * @brief Нормализует ядро (сумма коэффициентов = масштаб)
     * @param kernel Ядро для нормализации
     */
    static void normalizeKernel(std::vector<int32_t>& kernel);

    /**
     * @brief Применяет одномерное ядро по горизонтали
     * @param image Исходное изображение
     * @param kernel Ядро для применения (целочисленное, масштабированное)
     * @return Вектор с промежуточными результатами
     */
    static std::vector<uint8_t> applyHorizontalKernel(
        const ImageProcessor& image, 
        const std::vector<int32_t>& kernel
    );

    /**
     * @brief Применяет одномерное ядро по вертикали
     * @param horizontalResult Результат горизонтального применения
     * @param image Исходное изображение (для получения размеров)
     * @param kernel Ядро для применения (целочисленное, масштабированное)
     * @return Финальный результат размытия
     */
    static std::vector<uint8_t> applyVerticalKernel(
        const std::vector<uint8_t>& horizontalResult,
        const ImageProcessor& image,
        const std::vector<int32_t>& kernel
    );

private:
    double radius_;  // Радиус размытия
};

