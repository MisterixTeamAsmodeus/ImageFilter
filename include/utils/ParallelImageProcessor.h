#pragma once

#include <cstdint>
#include <functional>
#include <thread>
#include <vector>
#include <algorithm>

/**
 * @brief Утилита для параллельной обработки изображений
 * 
 * Предоставляет функции для распараллеливания обработки изображений
 * путем разделения работы на несколько потоков. Оптимизирует производительность
 * для больших изображений, используя все доступные ядра процессора.
 * 
 * Особенности:
 * - Автоматически определяет оптимальное количество потоков
 * - Разделяет изображение на горизонтальные полосы (строки)
 * - Обеспечивает безопасность потоков (thread-safe)
 */
class ParallelImageProcessor
{
public:
    /**
     * @brief Обрабатывает изображение построчно в параллельных потоках
     * 
     * Разделяет изображение на полосы и обрабатывает каждую полосу
     * в отдельном потоке. Функция обработки получает диапазон строк [start_row, end_row).
     * 
     * @param height Высота изображения в пикселях
     * @param processRowRange Функция обработки диапазона строк: void(int start_row, int end_row)
     * @param num_threads Количество потоков (0 = автоматическое определение)
     */
    static void processRowsParallel(
        int height,
        const std::function<void(int start_row, int end_row)>& processRowRange,
        int num_threads = 0
    );

    /**
     * @brief Получает оптимальное количество потоков для обработки
     * 
     * Возвращает количество потоков, равное количеству доступных ядер процессора,
     * или 1, если определение невозможно.
     * 
     * @return Количество потоков для использования
     */
    static int getOptimalThreadCount() noexcept;

private:
    /**
     * @brief Обрабатывает диапазон строк в отдельном потоке
     * 
     * Вспомогательная функция для выполнения обработки в потоке.
     * 
     * @param start_row Начальная строка (включительно)
     * @param end_row Конечная строка (исключительно)
     * @param processRowRange Функция обработки диапазона строк
     */
    static void processRowRangeThread(
        int start_row,
        int end_row,
        const std::function<void(int, int)>& processRowRange
    );
};



