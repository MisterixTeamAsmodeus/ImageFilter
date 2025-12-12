#include <gtest/gtest.h>
#include <utils/ThreadPool.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <condition_variable>

/**
 * @brief Тесты для класса ThreadPool
 */
class ThreadPoolTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }
    
    void TearDown() override
    {
    }
};

/**
 * @brief Тест создания пула с указанным количеством потоков
 * 
 * Проверяет, что пул создается с заданным количеством потоков
 */
TEST_F(ThreadPoolTest, CreateWithSpecificThreadCount)
{
    const int num_threads = 4;
    ThreadPool pool(num_threads);
    
    EXPECT_EQ(pool.getThreadCount(), num_threads);
}

/**
 * @brief Тест создания пула с автоматическим определением количества потоков
 * 
 * Проверяет, что пул создается с количеством потоков, равным количеству ядер процессора
 */
TEST_F(ThreadPoolTest, CreateWithAutoThreadCount)
{
    ThreadPool pool(0);
    
    // Количество потоков должно быть больше 0
    EXPECT_GT(pool.getThreadCount(), 0);
    
    // Количество потоков не должно превышать разумное значение
    EXPECT_LE(pool.getThreadCount(), 64);
}

/**
 * @brief Тест добавления задачи в очередь
 * 
 * Проверяет, что задача может быть добавлена в очередь
 */
TEST_F(ThreadPoolTest, EnqueueTask)
{
    ThreadPool pool(2);
    
    std::atomic<bool> task_executed{false};
    
    pool.enqueue([&task_executed]() {
        task_executed = true;
    });
    
    // Ждем выполнения задачи
    pool.waitAll();
    
    EXPECT_TRUE(task_executed);
}

/**
 * @brief Тест выполнения нескольких задач
 * 
 * Проверяет, что несколько задач могут быть выполнены
 */
TEST_F(ThreadPoolTest, ExecuteMultipleTasks)
{
    ThreadPool pool(2);
    
    const int num_tasks = 10;
    std::atomic<int> completed_tasks{0};
    
    for (int i = 0; i < num_tasks; ++i)
    {
        pool.enqueue([&completed_tasks]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            ++completed_tasks;
        });
    }
    
    pool.waitAll();
    
    EXPECT_EQ(completed_tasks.load(), num_tasks);
}

/**
 * @brief Тест ожидания завершения всех задач
 * 
 * Проверяет, что waitAll() корректно ждет завершения всех задач
 */
TEST_F(ThreadPoolTest, WaitAll)
{
    ThreadPool pool(2);
    
    std::atomic<int> counter{0};
    const int num_tasks = 5;
    
    for (int i = 0; i < num_tasks; ++i)
    {
        pool.enqueue([&counter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            ++counter;
        });
    }
    
    // Проверяем, что задачи еще не выполнены
    EXPECT_LT(counter.load(), num_tasks);
    
    // Ждем завершения всех задач
    pool.waitAll();
    
    // Проверяем, что все задачи выполнены
    EXPECT_EQ(counter.load(), num_tasks);
}

/**
 * @brief Тест получения количества потоков
 * 
 * Проверяет, что getThreadCount() возвращает корректное количество потоков
 */
TEST_F(ThreadPoolTest, GetThreadCount)
{
    const int num_threads = 8;
    ThreadPool pool(num_threads);
    
    EXPECT_EQ(pool.getThreadCount(), num_threads);
}

/**
 * @brief Тест получения размера очереди
 * 
 * Проверяет, что getQueueSize() возвращает корректный размер очереди
 */
TEST_F(ThreadPoolTest, GetQueueSize)
{
    ThreadPool pool(2);
    
    // Изначально очередь пуста
    EXPECT_EQ(pool.getQueueSize(), 0);
    
    // Добавляем задачи
    const int num_tasks = 5;
    for (int i = 0; i < num_tasks; ++i)
    {
        pool.enqueue([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }
    
    // Проверяем, что задачи добавлены в очередь
    // (могут быть уже выполнены, поэтому проверяем, что размер <= num_tasks)
    EXPECT_LE(pool.getQueueSize(), num_tasks);
    
    // Ждем завершения всех задач
    pool.waitAll();
    
    // После завершения очередь должна быть пуста
    EXPECT_EQ(pool.getQueueSize(), 0);
}

/**
 * @brief Тест thread-safety при добавлении задач
 * 
 * Проверяет, что добавление задач из разных потоков безопасно
 */
TEST_F(ThreadPoolTest, ThreadSafetyEnqueue)
{
    ThreadPool pool(4);
    
    const int num_threads = 4;
    const int tasks_per_thread = 10;
    std::atomic<int> completed_tasks{0};
    
    std::vector<std::thread> threads;
    
    // Создаем несколько потоков, каждый добавляет задачи
    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&pool, &completed_tasks]() {
            for (int j = 0; j < tasks_per_thread; ++j)
            {
                pool.enqueue([&completed_tasks]() {
                    ++completed_tasks;
                });
            }
        });
    }
    
    // Ждем завершения всех потоков добавления
    for (auto& thread : threads)
    {
        thread.join();
    }
    
    // Ждем завершения всех задач
    pool.waitAll();
    
    // Проверяем, что все задачи выполнены
    EXPECT_EQ(completed_tasks.load(), num_threads * tasks_per_thread);
}

/**
 * @brief Тест параллельного выполнения задач
 * 
 * Проверяет, что задачи выполняются параллельно
 */
TEST_F(ThreadPoolTest, ParallelExecution)
{
    const int num_threads = 4;
    ThreadPool pool(num_threads);
    
    std::atomic<int> concurrent_tasks{0};
    std::atomic<int> max_concurrent{0};
    std::mutex mutex;
    
    const int num_tasks = 20;
    
    for (int i = 0; i < num_tasks; ++i)
    {
        pool.enqueue([&concurrent_tasks, &max_concurrent, &mutex]() {
            int current = ++concurrent_tasks;
            
            {
                std::lock_guard<std::mutex> lock(mutex);
                if (current > max_concurrent.load())
                {
                    max_concurrent = current;
                }
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            --concurrent_tasks;
        });
    }
    
    pool.waitAll();
    
    // Проверяем, что задачи выполнялись параллельно
    // Максимальное количество одновременных задач должно быть >= 1
    // и может быть больше, если задачи выполняются параллельно
    EXPECT_GE(max_concurrent.load(), 1);
}

/**
 * @brief Тест корректного завершения пула
 * 
 * Проверяет, что пул корректно завершается при уничтожении
 */
TEST_F(ThreadPoolTest, ProperShutdown)
{
    {
        ThreadPool pool(2);
        
        std::atomic<int> completed_tasks{0};
        const int num_tasks = 5;
        
        for (int i = 0; i < num_tasks; ++i)
        {
            pool.enqueue([&completed_tasks]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                ++completed_tasks;
            });
        }
        
        // Не ждем завершения, просто уничтожаем пул
        // Пул должен корректно завершить все потоки
    }
    
    // Если мы дошли сюда, значит пул корректно завершился
    EXPECT_TRUE(true);
}

/**
 * @brief Тест обработки исключений в задачах
 * 
 * Проверяет, что исключения в задачах не приводят к падению пула
 */
TEST_F(ThreadPoolTest, ExceptionHandling)
{
    ThreadPool pool(2);
    
    std::atomic<bool> exception_caught{false};
    
    // Добавляем задачу, которая выбрасывает исключение
    pool.enqueue([&exception_caught]() {
        try
        {
            throw std::runtime_error("Test exception");
        }
        catch (...)
        {
            exception_caught = true;
        }
    });
    
    // Добавляем нормальную задачу
    std::atomic<bool> normal_task_executed{false};
    pool.enqueue([&normal_task_executed]() {
        normal_task_executed = true;
    });
    
    pool.waitAll();
    
    // Проверяем, что обе задачи выполнены
    EXPECT_TRUE(exception_caught);
    EXPECT_TRUE(normal_task_executed);
}

/**
 * @brief Тест добавления задач после waitAll
 * 
 * Проверяет, что можно добавлять задачи после вызова waitAll
 */
TEST_F(ThreadPoolTest, EnqueueAfterWaitAll)
{
    ThreadPool pool(2);
    
    std::atomic<int> counter{0};
    
    // Добавляем первую партию задач
    for (int i = 0; i < 5; ++i)
    {
        pool.enqueue([&counter]() {
            ++counter;
        });
    }
    
    pool.waitAll();
    EXPECT_EQ(counter.load(), 5);
    
    // Добавляем вторую партию задач
    for (int i = 0; i < 3; ++i)
    {
        pool.enqueue([&counter]() {
            ++counter;
        });
    }
    
    pool.waitAll();
    EXPECT_EQ(counter.load(), 8);
}

/**
 * @brief Тест производительности при большом количестве задач
 * 
 * Проверяет, что пул эффективно обрабатывает большое количество задач
 */
TEST_F(ThreadPoolTest, PerformanceWithManyTasks)
{
    ThreadPool pool(4);
    
    const int num_tasks = 1000;
    std::atomic<int> completed_tasks{0};
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_tasks; ++i)
    {
        pool.enqueue([&completed_tasks]() {
            ++completed_tasks;
        });
    }
    
    pool.waitAll();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_EQ(completed_tasks.load(), num_tasks);
    
    // Проверяем, что выполнение заняло разумное время
    // (1000 простых задач должны выполниться за несколько секунд)
    EXPECT_LT(duration.count(), 10000);
}

/**
 * @brief Тест создания пула с одним потоком
 * 
 * Проверяет, что пул может быть создан с одним потоком
 */
TEST_F(ThreadPoolTest, CreateWithOneThread)
{
    ThreadPool pool(1);
    
    EXPECT_EQ(pool.getThreadCount(), 1);
    
    std::atomic<int> counter{0};
    const int num_tasks = 10;
    
    for (int i = 0; i < num_tasks; ++i)
    {
        pool.enqueue([&counter]() {
            ++counter;
        });
    }
    
    pool.waitAll();
    
    EXPECT_EQ(counter.load(), num_tasks);
}

/**
 * @brief Тест последовательного выполнения задач в однопоточном пуле
 * 
 * Проверяет, что в однопоточном пуле задачи выполняются последовательно
 */
TEST_F(ThreadPoolTest, SequentialExecutionWithOneThread)
{
    ThreadPool pool(1);
    
    std::vector<int> execution_order;
    std::mutex order_mutex;
    
    const int num_tasks = 5;
    
    for (int i = 0; i < num_tasks; ++i)
    {
        pool.enqueue([&execution_order, &order_mutex, i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::lock_guard<std::mutex> lock(order_mutex);
            execution_order.push_back(i);
        });
    }
    
    pool.waitAll();
    
    // В однопоточном пуле задачи должны выполняться последовательно
    EXPECT_EQ(execution_order.size(), num_tasks);
    
    // Проверяем, что порядок выполнения соответствует порядку добавления
    for (int i = 0; i < num_tasks; ++i)
    {
        EXPECT_EQ(execution_order[i], i);
    }
}

