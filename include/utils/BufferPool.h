#pragma once

#include <utils/IBufferPool.h>
#include <mutex>
#include <vector>
#include <cstdint>
#include <cstddef>

/**
 * @brief Реализация пула буферов для переиспользования временных буферов
 * 
 * BufferPool предоставляет эффективный способ переиспользования буферов
 * для уменьшения количества аллокаций памяти при обработке изображений.
 * 
 * Особенности:
 * - Thread-safe: все операции синхронизированы через мьютекс
 * - Автоматическое управление размером пула
 * - Поддержка предварительного резервирования буферов
 * - Оптимизация для частых операций acquire/release
 * 
 * @note Рекомендуется использовать один экземпляр BufferPool на весь процесс
 * или на цепочку фильтров для максимальной эффективности.
 */
class BufferPool : public IBufferPool
{
public:
    /**
     * @brief Конструктор пула буферов
     * 
     * @param max_pool_size Максимальное количество буферов в пуле (0 = без ограничений)
     */
    explicit BufferPool(size_t max_pool_size = 0);

    /**
     * @brief Деструктор - освобождает все буферы
     */
    ~BufferPool() override = default;

    // Запрещаем копирование и присваивание
    BufferPool(const BufferPool&) = delete;
    BufferPool& operator=(const BufferPool&) = delete;

    /**
     * @brief Получить буфер из пула или создать новый
     * 
     * Если в пуле есть буфер подходящего размера, он будет возвращен.
     * В противном случае создается новый буфер.
     * 
     * @param size Минимальный размер буфера в байтах
     * @return std::vector<uint8_t> Буфер с зарезервированной памятью (capacity >= size)
     */
    std::vector<uint8_t> acquire(size_t size) override;

    /**
     * @brief Вернуть буфер в пул для переиспользования
     * 
     * Буфер будет добавлен в пул, если есть место и размер пула не превышен.
     * 
     * @param buffer Буфер для возврата в пул (будет перемещен)
     */
    void release(std::vector<uint8_t>&& buffer) override;

    /**
     * @brief Предварительно зарезервировать буферы определенного размера
     * 
     * Создает указанное количество буферов заданного размера и добавляет их в пул.
     * 
     * @param size Размер каждого буфера в байтах
     * @param count Количество буферов для резервирования
     */
    void reserve(size_t size, size_t count) override;

    /**
     * @brief Очистить пул, освободив все буферы
     */
    void clear() override;

    /**
     * @brief Получить количество буферов в пуле
     * 
     * @return size_t Общее количество буферов в пуле
     */
    size_t size() const override;

    /**
     * @brief Получить максимальный размер пула
     * 
     * @return size_t Максимальное количество буферов (0 = без ограничений)
     */
    [[nodiscard]] size_t getMaxPoolSize() const noexcept;

    /**
     * @brief Установить максимальный размер пула
     * 
     * @param max_size Максимальное количество буферов (0 = без ограничений)
     */
    void setMaxPoolSize(size_t max_size) noexcept;

    /**
     * @brief Получить общий объем памяти, зарезервированной в пуле
     * 
     * @return size_t Общий объем памяти в байтах (сумма capacity всех буферов)
     */
    size_t getTotalMemory() const override;

    /**
     * @brief Получить статистику использования пула
     * 
     * @param total_buffers Количество буферов в пуле
     * @param total_memory Общий объем зарезервированной памяти в байтах
     * @param largest_buffer Размер самого большого буфера в байтах
     * @param smallest_buffer Размер самого маленького буфера в байтах
     */
    void getStats(size_t& total_buffers, size_t& total_memory, 
                 size_t& largest_buffer, size_t& smallest_buffer) const override;

private:
    /**
     * @brief Находит буфер подходящего размера в пуле
     * 
     * Ищет буфер с capacity >= size. Если такого нет, возвращает пустой optional.
     * 
     * @param size Требуемый размер буфера
     * @return Индекс найденного буфера или -1 если не найден
     */
    int findSuitableBuffer(size_t size) const;

    mutable std::mutex mutex_;                    // Мьютекс для синхронизации
    std::vector<std::vector<uint8_t>> buffers_;    // Пул буферов
    size_t max_pool_size_;                         // Максимальный размер пула (0 = без ограничений)
};

