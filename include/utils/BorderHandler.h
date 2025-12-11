#pragma once

/**
 * @brief Класс для обработки границ изображения с различными стратегиями
 * 
 * Предоставляет единообразный интерфейс для обработки координат, выходящих за границы изображения.
 * Использует паттерн Strategy для выбора метода обработки границ.
 * 
 * Поддерживаемые стратегии:
 * - Mirror: отражение координат относительно границы
 * - Clamp: зажим координат к ближайшей границе
 * - Wrap: циклическое повторение (toroidal wrapping)
 * - Extend: расширение граничных значений
 */
class BorderHandler
{
public:
    /**
     * @brief Стратегии обработки границ изображения
     */
    enum class Strategy
    {
        Mirror,  ///< Отражение координат относительно границы (текущая реализация)
        Clamp,   ///< Зажим координат к границе [0, width-1] или [0, height-1]
        Wrap,    ///< Циклическое повторение (toroidal wrapping)
        Extend   ///< Расширение граничных значений
    };

    /**
     * @brief Конструктор
     * @param strategy Стратегия обработки границ (по умолчанию Mirror)
     */
    explicit BorderHandler(Strategy strategy = Strategy::Mirror) noexcept;

    /**
     * @brief Получить корректную координату X с учетом выбранной стратегии
     * @param x Исходная координата X (может быть вне диапазона [0, width))
     * @param width Ширина изображения
     * @return Корректная координата X в диапазоне [0, width)
     */
    [[nodiscard]] int getX(int x, int width) const noexcept;

    /**
     * @brief Получить корректную координату Y с учетом выбранной стратегии
     * @param y Исходная координата Y (может быть вне диапазона [0, height))
     * @param height Высота изображения
     * @return Корректная координата Y в диапазоне [0, height)
     */
    [[nodiscard]] int getY(int y, int height) const noexcept;

    /**
     * @brief Установить стратегию обработки границ
     * @param strategy Новая стратегия
     */
    void setStrategy(Strategy strategy) noexcept;

    /**
     * @brief Получить текущую стратегию обработки границ
     * @return Текущая стратегия
     */
    [[nodiscard]] Strategy getStrategy() const noexcept;

private:
    Strategy strategy_;  ///< Текущая стратегия обработки границ
};

