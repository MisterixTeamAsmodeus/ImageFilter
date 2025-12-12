#pragma once

#include <vector>
#include <cstdint>

/**
 * @brief Интерфейс для пула буферов
 * 
 * Определяет контракт для классов, предоставляющих пул буферов
 * для переиспользования временных буферов в фильтрах обработки изображений.
 * 
 * Использование интерфейса позволяет легко заменять реализации
 * и упрощает тестирование через моки.
 */
class IBufferPool
{
public:
    virtual ~IBufferPool() = default;

    /**
     * @brief Получить буфер из пула или создать новый
     * 
     * @param size Минимальный размер буфера в байтах
     * @return std::vector<uint8_t> Буфер с зарезервированной памятью
     */
    virtual std::vector<uint8_t> acquire(size_t size) = 0;

    /**
     * @brief Вернуть буфер в пул для переиспользования
     * 
     * @param buffer Буфер для возврата в пул
     */
    virtual void release(std::vector<uint8_t>&& buffer) = 0;

    /**
     * @brief Предварительно зарезервировать буферы определенного размера
     * 
     * @param size Размер каждого буфера в байтах
     * @param count Количество буферов для резервирования
     */
    virtual void reserve(size_t size, size_t count) = 0;

    /**
     * @brief Очистить пул, освободив все буферы
     */
    virtual void clear() = 0;

    /**
     * @brief Получить количество буферов в пуле
     * 
     * @return size_t Общее количество буферов в пуле
     */
    virtual size_t size() const = 0;

    /**
     * @brief Получить общий объем памяти, зарезервированной в пуле
     * 
     * @return size_t Общий объем памяти в байтах (сумма capacity всех буферов)
     */
    virtual size_t getTotalMemory() const = 0;

    /**
     * @brief Получить статистику использования пула
     * 
     * @param total_buffers Количество буферов в пуле
     * @param total_memory Общий объем зарезервированной памяти в байтах
     * @param largest_buffer Размер самого большого буфера в байтах
     * @param smallest_buffer Размер самого маленького буфера в байтах
     */
    virtual void getStats(size_t& total_buffers, size_t& total_memory, 
                         size_t& largest_buffer, size_t& smallest_buffer) const = 0;
};

