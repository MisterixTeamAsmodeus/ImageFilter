#pragma once

#include <memory>
#include <string>

#include <QWidget>

class BaseFilterConfigWidget;

/**
 * @brief Фабрика конфигурационных виджетов фильтров.
 *
 * Отвечает за сопоставление имени фильтра и конкретной реализации
 * виджета настроек, основанной на BaseFilterConfigWidget.
 */
class FilterWidgetFactory
{
public:
    /**
     * @brief Создает виджет параметров для указанного фильтра.
     *
     * Если для переданного имени фильтра не найдено подходящей
     * реализации, возвращается nullptr.
     *
     * @param filterName Имя фильтра.
     * @param parent Родительский виджет.
     * @return Умный указатель на созданный виджет или nullptr.
     */
    BaseFilterConfigWidget* create(const std::string& filterName, QWidget* parent) const;
};


