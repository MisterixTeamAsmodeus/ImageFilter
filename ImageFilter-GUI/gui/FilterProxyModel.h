#pragma once

#include <QSortFilterProxyModel>

/**
 * @brief Proxy модель для фильтрации дерева фильтров
 *
 * Обеспечивает фильтрацию элементов дерева по тексту поиска,
 * учитывая иерархическую структуру (категории и фильтры).
 * Категория показывается, если её название совпадает с поиском
 * или если хотя бы один из её фильтров совпадает.
 */
class FilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор proxy модели
     * @param parent Родительский объект
     */
    explicit FilterProxyModel(QObject* parent = nullptr);

    /**
     * @brief Устанавливает текст для фильтрации
     * @param filterText Текст для поиска
     */
    void setFilterText(const QString& filterText);

protected:
    /**
     * @brief Определяет, должен ли элемент быть показан
     * @param source_row Номер строки в исходной модели
     * @param source_parent Индекс родителя в исходной модели
     * @return true, если элемент должен быть показан
     */
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
    /**
     * @brief Проверяет, соответствует ли элемент поисковому запросу
     * @param index Индекс элемента в исходной модели
     * @return true, если элемент соответствует поиску
     */
    bool matchesFilter(const QModelIndex& index) const;

    QString filterText_;  ///< Текст для фильтрации
};

