#include <gui/FilterListModel.h>
#include <QStandardItem>
#include <QBrush>
#include <QColor>
#include <vector>
#include <utility>

FilterListModel::FilterListModel(QObject* parent)
    : QStandardItemModel(parent)
{
    initializeFilters();
}

void FilterListModel::initializeFilters()
{
    // Группируем фильтры по категориям
    struct FilterCategory
    {
        QString name;
        std::vector<std::pair<QString, QString>> filters;  // display name, filter name
    };

    std::vector<FilterCategory> categories = {
        {"Цветовые", {
            {"Оттенки серого", "grayscale"},
            {"Сепия", "sepia"},
            {"Инверсия", "invert"},
            {"Яркость", "brightness"},
            {"Контраст", "contrast"},
            {"Насыщенность", "saturation"}
        }},
        {"Геометрические", {
            {"Отразить по горизонтали", "flip_h"},
            {"Отразить по вертикали", "flip_v"},
            {"Поворот на 90°", "rotate90"}
        }},
        {"Края и детали", {
            {"Повышение резкости", "sharpen"},
            {"Детекция краёв", "edges"},
            {"Рельеф", "emboss"},
            {"Контур", "outline"}
        }},
        {"Размытие и шум", {
            {"Размытие по Гауссу", "blur"},
            {"Размытие по прямоугольнику", "box_blur"},
            {"Размытие движения", "motion_blur"},
            {"Медианный фильтр", "median"},
            {"Шум", "noise"}
        }},
        {"Стилистические", {
            {"Постеризация", "posterize"},
            {"Пороговая бинаризация", "threshold"},
            {"Виньетирование", "vignette"}
        }}
    };

    for (const auto& category : categories)
    {
        auto* categoryItem = new QStandardItem(category.name);
        categoryItem->setFlags(Qt::ItemIsEnabled);
        categoryItem->setBackground(QBrush(QColor(240, 240, 240)));
        categoryItem->setForeground(QBrush(QColor(100, 100, 100)));
        categoryItem->setSelectable(false);

        for (const auto& filter : category.filters)
        {
            auto* filterItem = new QStandardItem(filter.first);
            filterItem->setData(filter.second, Qt::UserRole);
            filterItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            categoryItem->appendRow(filterItem);
        }
        
        appendRow(categoryItem);
    }
}

