#pragma once

#include <QStandardItemModel>

/**
 * @brief Модель данных для списка фильтров
 *
 * Предоставляет структурированный список доступных фильтров,
 * сгруппированных по категориям. Автоматически инициализирует
 * все категории и фильтры при создании.
 */
class FilterListModel : public QStandardItemModel
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор модели списка фильтров
     * @param parent Родительский объект
     */
    explicit FilterListModel(QObject* parent = nullptr);

private:
    /**
     * @brief Инициализирует список фильтров
     *
     * Создает структуру категорий и фильтров, добавляя их в модель.
     */
    void initializeFilters();
};

