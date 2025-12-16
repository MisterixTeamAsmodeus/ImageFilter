#pragma once

#include <utils/IThreadPool.h>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

/**
 * @brief Пул потоков для переиспользования рабочих потоков
 * 
 * ThreadPool предоставляет эффективный способ выполнения задач в параллельных потоках
 * без создания и уничтожения потоков для каждой задачи. Это значительно снижает
 * накладные расходы на создание потоков и улучшает производительность.
 * 
 * Особенности:
 * - Автоматическое управление жизненным циклом потоков
 * - Thread-safe очередь задач
 * - Поддержка остановки и корректного завершения
 * - Настраиваемое количество потоков
 * - Реализует интерфейс IThreadPool для поддержки Dependency Injection
 * 
 * @note Все методы класса thread-safe
 */
class ThreadPool : public IThreadPool
{
public:
    /**
     * @brief Конструктор пула потоков
     * 
     * Создает указанное количество рабочих потоков. Если num_threads == 0,
     * использует количество доступных аппаратных потоков.
     * 
     * @param num_threads Количество потоков в пуле (0 = автоматическое определение)
     */
    explicit ThreadPool(int num_threads = 0);

    /**
     * @brief Деструктор - останавливает все потоки и ждет их завершения
     */
    ~ThreadPool() override;

    // Запрещаем копирование и присваивание
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    /**
     * @brief Добавляет задачу в очередь на выполнение
     * 
     * Задача будет выполнена одним из доступных рабочих потоков.
     * Метод не блокируется и возвращает управление сразу после добавления задачи.
     * 
     * @param task Функция для выполнения (void())
     */
    void enqueue(const std::function<void()>& task) override;

    /**
     * @brief Ждет завершения всех задач в очереди
     * 
     * Блокирует выполнение до тех пор, пока все задачи в очереди не будут выполнены.
     * Новые задачи, добавленные после вызова этого метода, не будут ожидаться.
     */
    void waitAll() override;

    /**
     * @brief Получает количество потоков в пуле
     * @return Количество рабочих потоков
     */
    [[nodiscard]] int getThreadCount() const noexcept override;

    /**
     * @brief Получает количество задач в очереди
     * @return Количество ожидающих задач
     */
    [[nodiscard]] size_t getQueueSize() const override;

private:
    /**
     * @brief Функция рабочего потока
     * 
     * Выполняется в каждом рабочем потоке. Поток постоянно извлекает задачи
     * из очереди и выполняет их, пока не будет остановлен.
     */
    void workerThread();

    std::vector<std::thread> workers_;              // Рабочие потоки
    std::queue<std::function<void()>> tasks_;       // Очередь задач
    std::mutex queue_mutex_;                        // Мьютекс для синхронизации доступа к очереди
    std::condition_variable condition_;             // Условная переменная для уведомления потоков
    std::condition_variable all_tasks_done_;        // Условная переменная для ожидания завершения всех задач
    std::atomic<size_t> active_tasks_{0};          // Счетчик активных задач
    std::atomic<bool> stop_{false};                 // Флаг остановки пула
};

