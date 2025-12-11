#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>

/**
 * @brief Утилиты для безопасных математических операций
 * 
 * Предоставляет функции для выполнения математических операций
 * с защитой от переполнения и деления на ноль.
 */
namespace SafeMath
{
    /**
     * @brief Безопасное умножение с проверкой переполнения
     * 
     * @param a Первый множитель
     * @param b Второй множитель
     * @param result Результат умножения (выходной параметр)
     * @return true если умножение успешно, false при переполнении
     */
    template<typename T>
    bool safeMultiply(T a, T b, T& result)
    {
        static_assert(std::is_integral_v<T>, "T must be an integral type");
        
        if (a == 0 || b == 0)
        {
            result = 0;
            return true;
        }
        
        // Проверка на переполнение
        if (a > std::numeric_limits<T>::max() / b)
        {
            return false; // Переполнение
        }
        
        if (std::is_signed_v<T> && a < std::numeric_limits<T>::min() / b)
        {
            return false; // Переполнение для отрицательных чисел
        }
        
        result = a * b;
        return true;
    }

    /**
     * @brief Безопасное деление с проверкой деления на ноль
     * 
     * @param a Делимое
     * @param b Делитель
     * @param result Результат деления (выходной параметр)
     * @return true если деление успешно, false при делении на ноль
     */
    template<typename T>
    bool safeDivide(T a, T b, T& result)
    {
        static_assert(std::is_integral_v<T>, "T must be an integral type");
        
        if (b == 0)
        {
            return false; // Деление на ноль
        }
        
        result = a / b;
        return true;
    }

    /**
     * @brief Безопасное сложение с проверкой переполнения
     * 
     * @param a Первое слагаемое
     * @param b Второе слагаемое
     * @param result Результат сложения (выходной параметр)
     * @return true если сложение успешно, false при переполнении
     */
    template<typename T>
    bool safeAdd(T a, T b, T& result)
    {
        static_assert(std::is_integral_v<T>, "T must be an integral type");
        
        if (std::is_signed_v<T>)
        {
            // Проверка на переполнение для знаковых типов
            if (b > 0 && a > std::numeric_limits<T>::max() - b)
            {
                return false; // Переполнение
            }
            if (b < 0 && a < std::numeric_limits<T>::min() - b)
            {
                return false; // Переполнение
            }
        }
        else
        {
            // Проверка на переполнение для беззнаковых типов
            if (a > std::numeric_limits<T>::max() - b)
            {
                return false; // Переполнение
            }
        }
        
        result = a + b;
        return true;
    }

    /**
     * @brief Ограничивает значение диапазоном [min_val, max_val]
     * 
     * @param value Значение для ограничения
     * @param min_val Минимальное значение
     * @param max_val Максимальное значение
     * @return Ограниченное значение
     */
    template<typename T>
    T clamp(T value, T min_val, T max_val)
    {
        if (value < min_val)
        {
            return min_val;
        }
        if (value > max_val)
        {
            return max_val;
        }
        return value;
    }
}

