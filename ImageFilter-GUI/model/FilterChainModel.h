#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <vector>
#include <map>
#include <string>

/**
 * @brief Модель цепочки фильтров для обработки изображений
 *
 * Управляет списком фильтров с их параметрами. Обеспечивает валидацию
 * (минимум 1 фильтр всегда должен присутствовать) и предоставляет методы
 * для добавления, удаления и изменения порядка фильтров.
 */
class FilterChainModel : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Структура для хранения информации о фильтре
     */
    struct FilterItem
    {
        std::string filterName;                          ///< Имя фильтра
        std::map<std::string, QVariant> parameters;      ///< Параметры фильтра

        /**
         * @brief Конструктор по умолчанию
         */
        FilterItem() = default;

        /**
         * @brief Конструктор с именем фильтра
         * @param name Имя фильтра
         */
        explicit FilterItem(const std::string& name) : filterName(name) {}
    };

    /**
     * @brief Конструктор модели цепочки фильтров
     * @param parent Родительский объект Qt
     */
    explicit FilterChainModel(QObject* parent = nullptr);

    /**
     * @brief Деструктор
     */
    ~FilterChainModel() override = default;

    /**
     * @brief Получает количество фильтров в цепочке
     * @return Количество фильтров
     */
    [[nodiscard]] size_t getFilterCount() const noexcept;

    /**
     * @brief Получает фильтр по индексу
     * @param index Индекс фильтра (0-based)
     * @return Константная ссылка на фильтр
     * @throws std::out_of_range если индекс невалиден
     */
    [[nodiscard]] const FilterItem& getFilter(size_t index) const;

    /**
     * @brief Получает фильтр по индексу (неконстантная версия)
     * @param index Индекс фильтра (0-based)
     * @return Ссылка на фильтр
     * @throws std::out_of_range если индекс невалиден
     */
    FilterItem& getFilter(size_t index);

    /**
     * @brief Добавляет фильтр в конец цепочки
     * @param filterName Имя фильтра
     * @return true если фильтр добавлен успешно
     */
    bool addFilter(const std::string& filterName);

    /**
     * @brief Добавляет фильтр с параметрами в конец цепочки
     * @param filterName Имя фильтра
     * @param parameters Параметры фильтра
     * @return true если фильтр добавлен успешно
     */
    bool addFilter(const std::string& filterName, const std::map<std::string, QVariant>& parameters);

    /**
     * @brief Вставляет фильтр по указанному индексу
     * @param index Индекс для вставки
     * @param filterName Имя фильтра
     * @param parameters Параметры фильтра
     * @return true если фильтр вставлен успешно
     */
    bool insertFilter(size_t index, const std::string& filterName, 
                     const std::map<std::string, QVariant>& parameters = {});

    /**
     * @brief Удаляет фильтр по индексу
     * @param index Индекс фильтра для удаления
     * @return true если фильтр удален успешно (нельзя удалить последний фильтр)
     */
    bool removeFilter(size_t index);

    /**
     * @brief Перемещает фильтр вверх по цепочке
     * @param index Индекс фильтра для перемещения
     * @return true если фильтр перемещен успешно
     */
    bool moveFilterUp(size_t index);

    /**
     * @brief Перемещает фильтр вниз по цепочке
     * @param index Индекс фильтра для перемещения
     * @return true если фильтр перемещен успешно
     */
    bool moveFilterDown(size_t index);

    /**
     * @brief Обновляет параметры фильтра
     * @param index Индекс фильтра
     * @param paramName Имя параметра
     * @param value Новое значение параметра
     * @return true если параметр обновлен успешно
     */
    bool updateFilterParameter(size_t index, const std::string& paramName, const QVariant& value);

    /**
     * @brief Изменяет тип фильтра (имя фильтра) по указанному индексу
     * @param index Индекс фильтра
     * @param newFilterName Новое имя фильтра
     * @return true если тип фильтра изменен успешно
     */
    bool changeFilterType(size_t index, const std::string& newFilterName);

    /**
     * @brief Очищает цепочку фильтров и добавляет фильтр по умолчанию
     */
    void clear();

    /**
     * @brief Проверяет валидность цепочки фильтров
     * @return true если цепочка валидна (минимум 1 фильтр)
     */
    [[nodiscard]] bool isValid() const noexcept;

    /**
     * @brief Получает все фильтры в цепочке
     * @return Константная ссылка на вектор фильтров
     */
    [[nodiscard]] const std::vector<FilterItem>& getFilters() const noexcept;

    /**
     * @brief Проверяет, можно ли удалить фильтр (нельзя удалить последний)
     * @return true если можно удалить фильтр
     */
    [[nodiscard]] bool canRemoveFilter() const noexcept;

signals:
    /**
     * @brief Сигнал об изменении цепочки фильтров
     */
    void chainChanged();

    /**
     * @brief Сигнал об изменении параметров фильтра
     * @param index Индекс измененного фильтра
     */
    void filterParameterChanged(size_t index);

private:
    std::vector<FilterItem> filters_;    ///< Список фильтров в цепочке
};

