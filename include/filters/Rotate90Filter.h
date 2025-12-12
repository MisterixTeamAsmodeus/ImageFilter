#pragma once

#include <filters/IFilter.h>

// Forward declaration
class IBufferPool;

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
     * @param buffer_pool Пул буферов для переиспользования временных буферов (опционально)
     */
    explicit Rotate90Filter(bool clockwise = true, IBufferPool* buffer_pool = nullptr) 
        : buffer_pool_(buffer_pool), clockwise_(clockwise) {}

    /**
     * @brief Применяет фильтр поворота к изображению
     * @param image Обрабатываемое изображение
     * @return true если фильтр применен успешно
     */
    FilterResult apply(ImageProcessor& image) override;

    std::string getName() const override;
    std::string getDescription() const override;
    std::string getCategory() const override;

private:
    IBufferPool* buffer_pool_;  // Пул буферов для переиспользования (может быть nullptr)
    bool clockwise_;  // Направление поворота
};



