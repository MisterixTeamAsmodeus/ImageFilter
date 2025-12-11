#include <utils/BorderHandler.h>
#include <algorithm>

namespace {
    /**
     * @brief Обработка координаты с использованием стратегии Mirror
     * @param coord Координата для обработки
     * @param max Максимальное значение координаты (не включительно)
     * @return Обработанная координата
     */
    [[nodiscard]] int handleMirror(int coord, int max) noexcept
    {
        if (coord < 0)
        {
            return -coord;
        }
        if (coord >= max)
        {
            return 2 * max - coord - 1;
        }
        return coord;
    }

    /**
     * @brief Обработка координаты с использованием стратегии Clamp
     * @param coord Координата для обработки
     * @param max Максимальное значение координаты (не включительно)
     * @return Обработанная координата
     */
    [[nodiscard]] int handleClamp(int coord, int max) noexcept
    {
        return std::clamp(coord, 0, max - 1);
    }

    /**
     * @brief Обработка координаты с использованием стратегии Wrap
     * @param coord Координата для обработки
     * @param max Максимальное значение координаты (не включительно)
     * @return Обработанная координата
     */
    [[nodiscard]] int handleWrap(int coord, int max) noexcept
    {
        if (max <= 0)
        {
            return 0;
        }

        // Обрабатываем отрицательные координаты
        while (coord < 0)
        {
            coord += max;
        }

        // Обрабатываем координаты >= max
        return coord % max;
    }

    /**
     * @brief Обработка координаты с использованием стратегии Extend
     * @param coord Координата для обработки
     * @param max Максимальное значение координаты (не включительно)
     * @return Обработанная координата
     */
    [[nodiscard]] int handleExtend(int coord, int max) noexcept
    {
        if (coord < 0)
        {
            return 0;
        }
        if (coord >= max)
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
    __builtin_unreachable();
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
    __builtin_unreachable();
}

void BorderHandler::setStrategy(BorderHandler::Strategy strategy) noexcept
{
    strategy_ = strategy;
}

BorderHandler::Strategy BorderHandler::getStrategy() const noexcept
{
    return strategy_;
}

