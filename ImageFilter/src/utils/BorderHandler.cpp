#include <utils/BorderHandler.h>
#include <algorithm>

namespace {
    /**
     * @brief Обработка координаты с использованием стратегии Mirror
     * @param coord Координата для обработки
     * @param max Максимальное значение координаты (не включительно)
     * @return Обработанная координата
     * 
     * Оптимизированная версия: использует unsigned сравнение для быстрой проверки диапазона
     */
    [[nodiscard]] inline int handleMirror(int coord, int max) noexcept
    {
        // Быстрая проверка: если координата в допустимом диапазоне, возвращаем как есть
        // Используем unsigned сравнение для проверки 0 <= coord < max за одну операцию
        if (static_cast<unsigned int>(coord) < static_cast<unsigned int>(max))
        {
            return coord;
        }
        
        // Обработка отрицательных координат
        if (coord < 0)
        {
            return -coord;
        }
        
        // Обработка координат >= max
        return 2 * max - coord - 1;
    }

    /**
     * @brief Обработка координаты с использованием стратегии Clamp
     * @param coord Координата для обработки
     * @param max Максимальное значение координаты (не включительно)
     * @return Обработанная координата
     */
    [[nodiscard]] inline int handleClamp(int coord, int max) noexcept
    {
        return std::clamp(coord, 0, max - 1);
    }

    /**
     * @brief Обработка координаты с использованием стратегии Wrap
     * @param coord Координата для обработки
     * @param max Максимальное значение координаты (не включительно)
     * @return Обработанная координата
     * 
     * Оптимизированная версия: использует модульную арифметику вместо цикла
     */
    [[nodiscard]] inline int handleWrap(int coord, int max) noexcept
    {
        if (max <= 0)
        {
            return 0;
        }

        // Оптимизация: используем модульную арифметику для отрицательных чисел
        // Для отрицательных чисел: ((coord % max) + max) % max
        // Для положительных: coord % max
        int result = coord % max;
        // Если результат отрицательный, добавляем max
        return (result < 0) ? (result + max) : result;
    }

    /**
     * @brief Обработка координаты с использованием стратегии Extend
     * @param coord Координата для обработки
     * @param max Максимальное значение координаты (не включительно)
     * @return Обработанная координата
     * 
     * Оптимизированная версия: идентична Clamp, но оставлена для ясности API
     */
    [[nodiscard]] inline int handleExtend(int coord, int max) noexcept
    {
        // Оптимизация: используем битовую операцию для быстрой проверки знака
        if (coord < 0)
        {
            return 0;
        }
        // Используем unsigned сравнение для проверки >= max
        if (static_cast<unsigned int>(coord) >= static_cast<unsigned int>(max))
        {
            return max - 1;
        }
        return coord;
    }
}

BorderHandler::BorderHandler(BorderHandler::Strategy strategy) noexcept
    : strategy_(strategy)
{
}

int BorderHandler::getX(int x, int width) const noexcept
{
    if (width <= 0)
    {
        return 0;
    }

    switch (strategy_)
    {
    case Strategy::Mirror:
        return handleMirror(x, width);
    case Strategy::Clamp:
        return handleClamp(x, width);
    case Strategy::Wrap:
        return handleWrap(x, width);
    case Strategy::Extend:
        return handleExtend(x, width);
    }
}

int BorderHandler::getY(int y, int height) const noexcept
{
    if (height <= 0)
    {
        return 0;
    }

    switch (strategy_)
    {
    case Strategy::Mirror:
        return handleMirror(y, height);
    case Strategy::Clamp:
        return handleClamp(y, height);
    case Strategy::Wrap:
        return handleWrap(y, height);
    case Strategy::Extend:
        return handleExtend(y, height);
    }
}

void BorderHandler::setStrategy(BorderHandler::Strategy strategy) noexcept
{
    strategy_ = strategy;
}

BorderHandler::Strategy BorderHandler::getStrategy() const noexcept
{
    return strategy_;
}

