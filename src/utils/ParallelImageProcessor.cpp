#include <utils/ParallelImageProcessor.h>
#include <thread>

void ParallelImageProcessor::processRowsParallel(
    int height,
    const std::function<void(int start_row, int end_row)>& processRowRange,
    int num_threads
)
{
    if (height <= 0)
    {
        return;
    }

    // Определяем количество потоков
    if (num_threads <= 0)
    {
        num_threads = getOptimalThreadCount();
    }

    // Если изображение маленькое или только один поток, обрабатываем последовательно
    if (num_threads == 1 || height < num_threads)
    {
        processRowRange(0, height);
        return;
    }

    // Вычисляем количество строк на поток
    const auto rows_per_thread = (height + num_threads - 1) / num_threads;

    // Создаем и запускаем потоки
    std::vector<std::thread> threads;
    threads.reserve(static_cast<size_t>(num_threads));

    for (int i = 0; i < num_threads; ++i)
    {
        const auto start_row = i * rows_per_thread;
        const auto end_row = std::min(start_row + rows_per_thread, height);

        // Если диапазон пустой, пропускаем поток
        if (start_row >= height)
        {
            break;
        }

        threads.emplace_back(processRowRangeThread, start_row, end_row, processRowRange);
    }

    // Ждем завершения всех потоков
    for (auto& thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
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

void ParallelImageProcessor::processRowRangeThread(
    int start_row,
    int end_row,
    const std::function<void(int, int)>& processRowRange
)
{
    processRowRange(start_row, end_row);
}



