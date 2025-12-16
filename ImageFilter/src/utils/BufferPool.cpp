#include <utils/BufferPool.h>
#include <algorithm>
#include <cstddef>

BufferPool::BufferPool(size_t max_pool_size)
    : max_pool_size_(max_pool_size)
{
}

std::vector<uint8_t> BufferPool::acquire(size_t size)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // Ищем подходящий буфер в пуле
    const int index = findSuitableBuffer(size);
    
    if (index >= 0)
    {
        // Найден подходящий буфер - извлекаем его из пула
        std::vector<uint8_t> buffer = std::move(buffers_[static_cast<size_t>(index)]);
        
        // Удаляем буфер из пула, перемещая последний элемент на его место
        buffers_[static_cast<size_t>(index)] = std::move(buffers_.back());
        buffers_.pop_back();
        
        // Убеждаемся, что буфер имеет достаточный размер
        if (buffer.size() < size)
        {
            buffer.resize(size);
        }
        
        return buffer;
    }
    
    // Подходящего буфера нет - создаем новый
    std::vector<uint8_t> buffer;
    buffer.reserve(size);
    buffer.resize(size);
    
    return buffer;
}

void BufferPool::release(std::vector<uint8_t>&& buffer)
{
    if (buffer.empty())
    {
        // Пустые буферы не добавляем в пул
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Проверяем ограничение размера пула
    if (max_pool_size_ > 0 && buffers_.size() >= max_pool_size_)
    {
        // Пул переполнен - не добавляем буфер
        return;
    }
    
    // Очищаем буфер, но сохраняем зарезервированную память
    buffer.clear();
    
    // Добавляем буфер в пул
    buffers_.push_back(std::move(buffer));
}

void BufferPool::reserve(size_t size, size_t count)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    const size_t current_size = buffers_.size();
    const size_t target_size = current_size + count;
    
    // Учитываем ограничение размера пула
    const size_t max_allowed = (max_pool_size_ == 0) ? target_size : 
                               std::min(target_size, max_pool_size_);
    const size_t to_create = (max_allowed > current_size) ? (max_allowed - current_size) : 0;
    
    // Резервируем место для новых буферов
    buffers_.reserve(max_allowed);
    
    // Создаем буферы
    for (size_t i = 0; i < to_create; ++i)
    {
        std::vector<uint8_t> buffer;
        buffer.reserve(size);
        buffer.resize(size);
        buffers_.push_back(std::move(buffer));
    }
}

void BufferPool::clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    buffers_.clear();
}

size_t BufferPool::size() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return buffers_.size();
}

size_t BufferPool::getMaxPoolSize() const noexcept
{
    return max_pool_size_;
}

void BufferPool::setMaxPoolSize(size_t max_size) noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    max_pool_size_ = max_size;
    
    // Если текущий размер пула превышает новый максимум, удаляем лишние буферы
    if (max_pool_size_ > 0 && buffers_.size() > max_pool_size_)
    {
        buffers_.resize(max_pool_size_);
        buffers_.shrink_to_fit();
    }
}

int BufferPool::findSuitableBuffer(size_t size) const
{
    // Ищем буфер с capacity >= size
    // Предпочитаем буферы с минимальным превышением размера для экономии памяти
    int best_index = -1;
    size_t best_capacity = 0;
    
    for (size_t i = 0; i < buffers_.size(); ++i)
    {
        const size_t capacity = buffers_[i].capacity();
        if (capacity >= size)
        {
            // Найден подходящий буфер
            if (best_index < 0 || capacity < best_capacity)
            {
                // Это лучший вариант (минимальное превышение размера)
                best_index = static_cast<int>(i);
                best_capacity = capacity;
            }
        }
    }
    
    return best_index;
}

size_t BufferPool::getTotalMemory() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t total = 0;
    for (const auto& buffer : buffers_)
    {
        total += buffer.capacity();
    }
    
    return total;
}

void BufferPool::getStats(size_t& total_buffers, size_t& total_memory, 
                         size_t& largest_buffer, size_t& smallest_buffer) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    total_buffers = buffers_.size();
    total_memory = 0;
    largest_buffer = 0;
    smallest_buffer = 0;
    
    if (buffers_.empty())
    {
        return;
    }
    
    smallest_buffer = buffers_[0].capacity();
    
    for (const auto& buffer : buffers_)
    {
        const size_t capacity = buffer.capacity();
        total_memory += capacity;
        
        if (capacity > largest_buffer)
        {
            largest_buffer = capacity;
        }
        
        if (capacity < smallest_buffer)
        {
            smallest_buffer = capacity;
        }
    }
}

