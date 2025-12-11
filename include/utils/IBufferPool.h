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
};

