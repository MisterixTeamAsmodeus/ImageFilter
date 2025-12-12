#include <cli/FilterInfoDisplay.h>
#include <utils/FilterFactory.h>
#include <filters/IFilter.h>
#include <CLI/CLI.hpp>
#include <iostream>
#include <map>
#include <vector>
#include <iomanip>

void FilterInfoDisplay::printFilterList(CLI::App& app)
{
    auto& factory = FilterFactory::getInstance();
    auto filter_names = factory.getRegisteredFilters();
    
    // Группируем фильтры по категориям
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> categories;
    
    for (const auto& name : filter_names)
    {
        auto filter = factory.create(name, app);
        if (filter)
        {
            categories[filter->getCategory()].emplace_back(name, filter->getDescription());
        }
    }
    
    std::cout << "Доступные фильтры:" << std::endl;
    std::cout << std::endl;
    
    // Выводим фильтры по категориям
    for (const auto& [category, filters] : categories)
    {
        std::cout << category << ":" << std::endl;
        for (const auto& [name, description] : filters)
        {
            std::cout << "  " << std::setw(12) << std::left << name << " - " << description << std::endl;
        }
        std::cout << std::endl;
    }
}

void FilterInfoDisplay::printFilterInfo(const std::string& filter_name, CLI::App& app)
{
    auto& factory = FilterFactory::getInstance();
    
    if (!factory.isRegistered(filter_name))
    {
        std::cout << "Неизвестный фильтр: " << filter_name << std::endl;
        std::cout << "Используйте --list-filters для списка доступных фильтров" << std::endl;
        return;
    }
    
    auto filter = factory.create(filter_name, app);
    if (!filter)
    {
        std::cout << "Ошибка: не удалось создать фильтр " << filter_name << std::endl;
        return;
    }
    
    std::cout << "Фильтр: " << filter->getName() << std::endl;
    std::cout << "Описание: " << filter->getDescription() << std::endl;
    std::cout << "Категория: " << filter->getCategory() << std::endl;
}

