#include <utils/ThreadPool.h>
#include <utils/IThreadPool.h>
#include <algorithm>
#include <thread>

IThreadPool::~IThreadPool() = default;

namespace
{
    /**
     * @brief Получает оптимальное количество потоков для пула
     * 
     * Вспомогательная функция для определения количества потоков.
     * Находится в anonymous namespace согласно правилу: не использовать
     * статические методы в приватном API, если есть cpp файл.
     * 
     * @return Количество потоков (минимум 1)
     */
    int getOptimalThreadCount() noexcept
    {
        const auto hardware_threads = static_cast<int>(std::thread::hardware_concurrency());
        
        // Если не удалось определить, используем 1 поток
        if (hardware_threads == 0)
        {
            return 1;
        }

        return hardware_threads;
    }
}

ThreadPool::ThreadPool(int num_threads)
{
    // Определяем количество потоков
    if (num_threads <= 0)
    {
        num_threads = getOptimalThreadCount();
    }

    // Создаем рабочие потоки
    workers_.reserve(static_cast<size_t>(num_threads));
    for (int i = 0; i < num_threads; ++i)
    {
        workers_.emplace_back(&ThreadPool::workerThread, this);
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }

    // Уведомляем все потоки о необходимости остановки
    condition_.notify_all();

    // Ждем завершения всех потоков
    for (auto& worker : workers_)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}

void ThreadPool::enqueue(const std::function<void()>& task)
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        
        // Не добавляем задачи после остановки
        if (stop_)
        {
            return;
        }

        tasks_.push(task);
    }

    // Уведомляем один из ожидающих потоков
    condition_.notify_one();
}

void ThreadPool::waitAll()
{
    std::unique_lock<std::mutex> lock(queue_mutex_);
    
    // Ждем, пока все задачи не будут выполнены
    all_tasks_done_.wait(lock, [this] {
        return tasks_.empty() && active_tasks_ == 0;
    });
}

int ThreadPool::getThreadCount() const noexcept
{
    return static_cast<int>(workers_.size());
}

size_t ThreadPool::getQueueSize() const
{
    std::unique_lock<std::mutex> lock(const_cast<std::mutex&>(queue_mutex_));
    return tasks_.size();
}

void ThreadPool::workerThread()
{
    while (true)
    {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(queue_mutex_);

            // Ждем задачи или сигнала остановки
            condition_.wait(lock, [this] {
                return stop_ || !tasks_.empty();
            });

            // Если остановка запрошена и очередь пуста, выходим
            if (stop_ && tasks_.empty())
            {
                return;
            }

            // Извлекаем задачу из очереди
            if (!tasks_.empty())
            {
                task = tasks_.front();
                tasks_.pop();
                ++active_tasks_;
            }
        }

        // Выполняем задачу вне блокировки
        if (task)
        {
            task();
            
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                --active_tasks_;
                
                // Уведомляем, если все задачи выполнены
                if (tasks_.empty() && active_tasks_ == 0)
                {
                    all_tasks_done_.notify_all();
                }
            }
        }
    }
}

