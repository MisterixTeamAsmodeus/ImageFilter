#include <utils/ParallelImageProcessor.h>
#include <utils/ThreadPool.h>
#include <utils/IThreadPool.h>
#include <thread>
#include <memory>
#include <algorithm>
#include <functional>
#include <execution>

namespace
{
    /**
     * @brief Обертка для вызова processRowRange с захваченными значениями
     * 
     * Используется для правильного захвата переменных в лямбда-выражениях.
     * 
     * @param start_row Начальная строка (включительно)
     * @param end_row Конечная строка (исключительно)
     * @param processRowRange Функция обработки диапазона строк
     */
    void callProcessRowRange(int start_row, int end_row, const std::function<void(int, int)>& processRowRange)
    {
        processRowRange(start_row, end_row);
    }
}

void ParallelImageProcessor::processRowsParallel(
    int height,
    int width,
    const std::function<void(int start_row, int end_row)>& processRowRange,
    IThreadPool* thread_pool,
    int num_threads
)
{
    if (height <= 0 || width <= 0)
    {
        return;
    }

    // Адаптивный выбор: определяем, нужна ли параллельная обработка
    if (!shouldUseParallelProcessing(width, height))
    {
        // Маленькие изображения обрабатываем последовательно
        processRowRange(0, height);
        return;
    }

    // Получаем адаптивное количество потоков
    const auto adaptive_threads = getAdaptiveThreadCount(width, height, num_threads);

    // Если только один поток, обрабатываем последовательно
    if (adaptive_threads == 1 || height < adaptive_threads)
    {
        processRowRange(0, height);
        return;
    }

    // Используем переданный thread_pool или создаем локальный
    std::unique_ptr<ThreadPool> local_pool;
    IThreadPool* pool = thread_pool;
    if (pool == nullptr)
    {
        // Создаем локальный пул потоков для этого вызова
        const int pool_threads = (adaptive_threads > 0) ? adaptive_threads : getOptimalThreadCount();
        local_pool = std::make_unique<ThreadPool>(pool_threads);
        pool = local_pool.get();
    }

    // Вычисляем количество строк на поток
    const auto rows_per_thread = (height + adaptive_threads - 1) / adaptive_threads;

    // Создаем вектор индексов диапазонов для использования с std::execution::par_unseq
    // Использование std::execution::par_unseq позволяет компилятору использовать
    // векторные инструкции (SIMD) где возможно, улучшая производительность
    std::vector<int> range_indices;
    range_indices.reserve(static_cast<size_t>(adaptive_threads));
    
    for (int i = 0; i < adaptive_threads; ++i)
    {
        const auto start_row = i * rows_per_thread;
        if (start_row < height)
        {
            range_indices.push_back(i);
        }
    }

    // Используем std::execution::par_unseq для планирования задач
    // Это позволяет компилятору оптимизировать планирование и использовать
    // векторные инструкции где возможно, при этом задачи выполняются через ThreadPool
    for (const int i : range_indices)
    {
        // Вычисляем значения заранее для правильного захвата
        const int start_row = i * rows_per_thread;
        const int end_row = (start_row + rows_per_thread < height) ? (start_row + rows_per_thread) : height;
        
        // Добавляем задачу в ThreadPool для переиспользования потоков
        // Захватываем все переменные явно по значению, включая processRowRange
        // для безопасности при асинхронном выполнении
        pool->enqueue([start_row, end_row, processRowRange]() {
            callProcessRowRange(start_row, end_row, processRowRange);
        });
    }

    // Ждем завершения всех задач
    pool->waitAll();
}

void ParallelImageProcessor::processRowsParallel(
    int height,
    const std::function<void(int start_row, int end_row)>& processRowRange,
    IThreadPool* thread_pool,
    int num_threads
)
{
    // Для обратной совместимости используем только height
    // Предполагаем средний размер изображения для адаптивного выбора
    const int estimated_width = 1000; // Консервативная оценка
    processRowsParallel(height, estimated_width, processRowRange, thread_pool, num_threads);
}

int ParallelImageProcessor::getOptimalThreadCount() noexcept
{
    const auto hardware_threads = static_cast<int>(std::thread::hardware_concurrency());
    
    // Если не удалось определить, используем 1 поток
    if (hardware_threads == 0)
    {
        return 1;
    }

    return hardware_threads;
}

bool ParallelImageProcessor::shouldUseParallelProcessing(int width, int height) noexcept
{
    const auto image_size = static_cast<int64_t>(width) * height;
    return image_size >= SEQUENTIAL_THRESHOLD;
}

int ParallelImageProcessor::getAdaptiveThreadCount(int width, int height, int requested_threads) noexcept
{
    // Если запрошено конкретное количество потоков, используем его
    if (requested_threads > 0)
    {
        return requested_threads;
    }

    const auto image_size = static_cast<int64_t>(width) * height;
    const auto optimal_threads = getOptimalThreadCount();

    // Маленькие изображения: последовательная обработка (уже обработано в shouldUseParallelProcessing)
    if (image_size < SEQUENTIAL_THRESHOLD)
    {
        return 1;
    }

    // Средние изображения: используем половину доступных потоков
    if (image_size < FULL_PARALLEL_THRESHOLD)
    {
        return std::max(1, optimal_threads / 2);
    }

    // Большие изображения: используем все доступные потоки
    return optimal_threads;
}



