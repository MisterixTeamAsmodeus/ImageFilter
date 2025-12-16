#include <gui/FilterProxyModel.h>
#include <QModelIndex>

FilterProxyModel::FilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
    , filterText_()
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void FilterProxyModel::setFilterText(const QString& filterText)
{
    filterText_ = filterText.toLower();
    invalidateFilter();
}

bool FilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (filterText_.isEmpty())
    {
        return true;
    }

    const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    if (!index.isValid())
    {
        return false;
    }

    // Проверяем сам элемент
    if (matchesFilter(index))
    {
        return true;
    }

    // Если это категория, проверяем её дочерние элементы
    const int rowCount = sourceModel()->rowCount(index);
    for (int i = 0; i < rowCount; ++i)
    {
        if (filterAcceptsRow(i, index))
        {
            return true;
        }
    }

    return false;
}

bool FilterProxyModel::matchesFilter(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return false;
    }

    const QString text = sourceModel()->data(index, Qt::DisplayRole).toString().toLower();
    return text.contains(filterText_);
}

