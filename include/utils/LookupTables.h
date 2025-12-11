#pragma once

#include <cstdint>
#include <array>
#include <mutex>

/**
 * @brief Утилита для предвычисленных lookup tables
 * 
 * Содержит статические lookup tables для часто используемых математических функций,
 * что позволяет избежать дорогостоящих вычислений в runtime.
 * 
 * Принципы:
 * - Все таблицы инициализируются один раз при первом использовании
 * - Thread-safe инициализация
 * - Оптимизированы для производительности
 */
class LookupTables {
public:
    /**
     * @brief Инициализирует все lookup tables
     * 
     * Должна быть вызвана перед использованием таблиц.
     * Thread-safe: можно вызывать из разных потоков.
     */
    static void initialize();

    /**
     * @brief Получает значение sin для угла в градусах
     * @param angle_degrees Угол в градусах [0, 360)
     * @return Значение sin, масштабированное на 65536 для целочисленной арифметики
     */
    static int32_t sinScaled(int angle_degrees) noexcept;

    /**
     * @brief Получает значение cos для угла в градусах
     * @param angle_degrees Угол в градусах [0, 360)
     * @return Значение cos, масштабированное на 65536 для целочисленной арифметики
     */
    static int32_t cosScaled(int angle_degrees) noexcept;

    /**
     * @brief Получает значение sin для угла в градусах (double)
     * @param angle_degrees Угол в градусах [0, 360)
     * @return Значение sin
     */
    static double sin(int angle_degrees) noexcept;

    /**
     * @brief Получает значение cos для угла в градусах (double)
     * @param angle_degrees Угол в градусах [0, 360)
     * @return Значение cos
     */
    static double cos(int angle_degrees) noexcept;

    /**
     * @brief Получает значение exp(-x) для x >= 0
     * 
     * Используется для вычисления функции Гаусса.
     * Таблица содержит значения для x в диапазоне [0, 20] с шагом 0.01.
     * 
     * @param x Аргумент (должен быть >= 0)
     * @return Значение exp(-x)
     */
    static double expNegative(double x) noexcept;

    /**
     * @brief Получает значение sqrt для целого числа
     * 
     * Таблица содержит значения для x в диапазоне [0, 10000].
     * Для значений вне диапазона используется std::sqrt.
     * 
     * @param x Целое число (должно быть >= 0)
     * @return Значение sqrt(x)
     */
    static double sqrtInt(int x) noexcept;

    /**
     * @brief Получает значение sqrt для целого числа (целочисленный результат)
     * 
     * Возвращает округленное значение sqrt(x) * 65536 для целочисленной арифметики.
     * 
     * @param x Целое число (должно быть >= 0)
     * @return Значение sqrt(x) * 65536
     */
    static int32_t sqrtIntScaled(int x) noexcept;


};

