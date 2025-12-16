#pragma once

#include "BatchProcessor.h"
#include <chrono>
#include <string>

/**
 * @brief Класс для отображения прогресса обработки
 * 
 * Отвечает за:
 * - Форматирование и отображение прогресса обработки файлов
 * - Форматирование времени
 */
class ProgressDisplay
{
public:
    /**
     * @brief Отображает прогресс обработки файла с улучшенной информацией
     * @param info Информация о прогрессе
     */
    static void displayProgress(const ProgressInfo& info);

private:
    /**
     * @brief Форматирует время в читаемый вид
     * @param seconds Количество секунд
     * @return Отформатированная строка времени
     */
    static std::string formatTime(std::chrono::seconds seconds);
};

