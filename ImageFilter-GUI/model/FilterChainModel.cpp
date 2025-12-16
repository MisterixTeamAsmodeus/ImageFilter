#include <model/FilterChainModel.h>
#include <stdexcept>

FilterChainModel::FilterChainModel(QObject* parent)
    : QObject(parent)
{
    // Добавляем фильтр по умолчанию для обеспечения валидности
    filters_.emplace_back("grayscale");
}

size_t FilterChainModel::getFilterCount() const noexcept
{
    return filters_.size();
}

const FilterChainModel::FilterItem& FilterChainModel::getFilter(size_t index) const
{
    if (index >= filters_.size())
    {
        throw std::out_of_range("Индекс фильтра вне допустимого диапазона");
    }
    return filters_[index];
}

FilterChainModel::FilterItem& FilterChainModel::getFilter(size_t index)
{
    if (index >= filters_.size())
    {
        throw std::out_of_range("Индекс фильтра вне допустимого диапазона");
    }
    return filters_[index];
}

bool FilterChainModel::addFilter(const std::string& filterName)
{
    return addFilter(filterName, {});
}

bool FilterChainModel::addFilter(const std::string& filterName, 
                                 const std::map<std::string, QVariant>& parameters)
{
    // Валидация: проверяем, что имя фильтра не пустое
    if (filterName.empty())
    {
        return false;
    }

    FilterItem item(filterName);
    item.parameters = parameters;
    filters_.push_back(item);

    emit chainChanged();
    return true;
}

bool FilterChainModel::insertFilter(size_t index, const std::string& filterName,
                                    const std::map<std::string, QVariant>& parameters)
{
    // Валидация: проверяем, что имя фильтра не пустое и индекс валиден
    if (filterName.empty() || index > filters_.size())
    {
        return false;
    }

    FilterItem item(filterName);
    item.parameters = parameters;
    filters_.insert(filters_.begin() + static_cast<ptrdiff_t>(index), item);

    emit chainChanged();
    return true;
}

bool FilterChainModel::removeFilter(size_t index)
{
    // Валидация: проверяем, что можно удалить фильтр (минимум 1 должен остаться)
    if (!canRemoveFilter() || index >= filters_.size())
    {
        return false;
    }

    filters_.erase(filters_.begin() + static_cast<ptrdiff_t>(index));
    emit chainChanged();
    return true;
}

bool FilterChainModel::moveFilterUp(size_t index)
{
    // Валидация: проверяем, что индекс валиден и фильтр не первый
    if (filters_.empty() || index == 0 || index >= filters_.size())
    {
        return false;
    }

    std::swap(filters_[index], filters_[index - 1]);
    emit chainChanged();
    return true;
}

bool FilterChainModel::moveFilterDown(size_t index)
{
    // Валидация: проверяем, что индекс валиден и фильтр не последний
    if (filters_.empty() || index >= filters_.size() - 1)
    {
        return false;
    }

    std::swap(filters_[index], filters_[index + 1]);
    emit chainChanged();
    return true;
}

bool FilterChainModel::updateFilterParameter(size_t index, const std::string& paramName, 
                                            const QVariant& value)
{
    // Валидация: проверяем, что индекс валиден и имя параметра не пустое
    if (filters_.empty() || index >= filters_.size() || paramName.empty())
    {
        return false;
    }

    filters_[index].parameters[paramName] = value;
    emit filterParameterChanged(index);
    emit chainChanged();
    return true;
}

bool FilterChainModel::changeFilterType(size_t index, const std::string& newFilterName)
{
    // Валидация: проверяем, что индекс валиден и новое имя фильтра не пустое
    if (filters_.empty() || index >= filters_.size() || newFilterName.empty())
    {
        return false;
    }

    // Если тип фильтра не изменился, ничего не делаем
    if (filters_[index].filterName == newFilterName)
    {
        return true;
    }

    // Изменяем тип фильтра и очищаем параметры (так как новый фильтр может иметь другие параметры)
    filters_[index].filterName = newFilterName;
    filters_[index].parameters.clear();

    emit chainChanged();
    return true;
}

void FilterChainModel::clear()
{
    filters_.clear();
    // Добавляем фильтр по умолчанию для обеспечения валидности
    filters_.emplace_back("grayscale");
    emit chainChanged();
}

bool FilterChainModel::isValid() const noexcept
{
    return !filters_.empty();
}

const std::vector<FilterChainModel::FilterItem>& FilterChainModel::getFilters() const noexcept
{
    return filters_;
}

bool FilterChainModel::canRemoveFilter() const noexcept
{
    // Нельзя удалить последний фильтр
    return filters_.size() > 1;
}

