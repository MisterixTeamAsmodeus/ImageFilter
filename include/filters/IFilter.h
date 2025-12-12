#pragma once

#include <utils/FilterResult.h>
#include <string>

class ImageProcessor;

/**
 * @brief Базовый интерфейс для всех фильтров изображений
 * 
 * Использует полиморфизм для единообразной работы с различными фильтрами.
 * Все фильтры должны наследоваться от этого интерфейса и реализовывать метод apply().
 * 
 * Особенности C++20:
 * - Использует виртуальный деструктор для корректного удаления объектов через указатель на базовый класс
 * - Позволяет работать с фильтрами через указатели на интерфейс (паттерн Strategy)
 * - Использует FilterResult для структурированной обработки ошибок
 * 
 * @example example_filter_usage.cpp
 * Пример использования фильтров:
 * @code{.cpp}
 * auto filter = FilterFactory::getInstance().create("grayscale", app);
 * if (filter) {
 *     filter->apply(processor);
 * }
 * @endcode
 * 
 * @example example_custom_filter.cpp
 * Пример создания пользовательского фильтра, наследующегося от IFilter
 */
class IFilter {
public:
    /**
     * @brief Виртуальный деструктор
     * 
     * Необходим для корректного удаления объектов через указатель на базовый класс.
     * Определение вынесено в .cpp файл для предотвращения создания vtable в каждой единице трансляции.
     */
    virtual ~IFilter();

    /**
     * @brief Применяет фильтр к изображению
     * @param image Обрабатываемое изображение
     * @return FilterResult с кодом ошибки и опциональным сообщением
     */
    virtual FilterResult apply(ImageProcessor& image) = 0;

    /**
     * @brief Возвращает имя фильтра
     * @return Строка с именем фильтра
     */
    [[nodiscard]] virtual std::string getName() const = 0;

    /**
     * @brief Возвращает описание фильтра
     * @return Строка с описанием фильтра
     */
    [[nodiscard]] virtual std::string getDescription() const = 0;

    /**
     * @brief Возвращает категорию фильтра
     * @return Строка с категорией (например, "Цветовой", "Геометрический", "Края и детали", "Размытие и шум", "Стилистический")
     */
    [[nodiscard]] virtual std::string getCategory() const = 0;

    /**
     * @brief Проверяет, может ли фильтр работать in-place (изменять данные напрямую без копирования)
     * 
     * In-place обработка возможна, если фильтр не требует чтения исходных значений соседних пикселей
     * во время записи результата. Это позволяет избежать создания временных буферов и копирования данных.
     * 
     * @return true если фильтр может работать in-place, false если требуется копирование данных
     */
    [[nodiscard]] virtual bool supportsInPlace() const noexcept { return false; }

};

