#pragma once

#include <functional>
#include <cstddef>

/**
 * @brief Интерфейс для пула потоков
 * 
 * Определяет контракт для классов, предоставляющих пул потоков
 * для выполнения задач в параллельных потоках.
 * 
 * Использование интерфейса позволяет легко заменять реализации
 * и упрощает тестирование через моки.
 */
class IThreadPool
{
public:
    virtual ~IThreadPool();

    /**
     * @brief Добавляет задачу в очередь на выполнение
     * 
     * @param task Функция для выполнения (void())
     */
    virtual void enqueue(const std::function<void()>& task) = 0;

    /**
     * @brief Ждет завершения всех задач в очереди
     */
    virtual void waitAll() = 0;

    /**
     * @brief Получает количество потоков в пуле
     * @return Количество рабочих потоков
     */
    virtual int getThreadCount() const noexcept = 0;

    /**
     * @brief Получает количество задач в очереди
     * @return Количество ожидающих задач
     */
    virtual size_t getQueueSize() const = 0;
};

