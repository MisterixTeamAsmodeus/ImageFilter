#pragma once

#include <filters/IFilter.h>
#include <utils/BorderHandler.h>

// Forward declaration
class IBufferPool;

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
     * @param length Длина размытия в пикселях (по умолчанию 10, должен быть > 0)
     * @param angle Угол направления размытия в градусах (0 = горизонтально, 90 = вертикально, по умолчанию 0)
     *              При некорректных значениях используются значения по умолчанию
     * @param borderStrategy Стратегия обработки границ (по умолчанию Mirror)
     * @param buffer_pool Пул буферов для переиспользования временных буферов (опционально)
     */
    explicit MotionBlurFilter(int length = 10, 
                              double angle = 0.0,
                              BorderHandler::Strategy borderStrategy = BorderHandler::Strategy::Mirror,
                              IBufferPool* buffer_pool = nullptr) 
        : angle_(angle), length_(length > 0 ? length : 10), border_handler_(borderStrategy), buffer_pool_(buffer_pool) {}

    /**
     * @brief Применяет фильтр размытия движения к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    FilterResult apply(ImageProcessor& image) override;

    std::string getName() const override;
    std::string getDescription() const override;
    std::string getCategory() const override;

private:
    double angle_;   // Угол направления размытия в градусах
    int length_;     // Длина размытия
    BorderHandler border_handler_;  // Обработчик границ
    IBufferPool* buffer_pool_;  // Пул буферов для переиспользования (может быть nullptr)
};



