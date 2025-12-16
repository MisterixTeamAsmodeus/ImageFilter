#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <vector>

// Forward declarations
class FilterChainModel;
class FilterItemWidget;
class QPushButton;

namespace Ui
{
    class FilterChainPanel;
}

/**
 * @brief Панель управления цепочкой фильтров
 *
 * Отображает список фильтров в цепочке и предоставляет интерфейс
 * для добавления, удаления и изменения порядка фильтров.
 */
class FilterChainPanel : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор панели цепочки фильтров
     * @param filterChainModel Модель цепочки фильтров
     * @param parent Родительский виджет
     */
    explicit FilterChainPanel(FilterChainModel* filterChainModel, QWidget* parent = nullptr);

    /**
     * @brief Деструктор
     */
    ~FilterChainPanel() override;

private slots:
    /**
     * @brief Обработчик нажатия кнопки "Добавить фильтр"
     */
    void onAddFilter();

    /**
     * @brief Обработчик изменения цепочки фильтров
     */
    void onChainChanged();

private:

    /**
     * @brief Обновляет отображение списка фильтров
     */
    void updateFilterList();

    /**
     * @brief Показывает диалог выбора фильтра
     * @return Имя выбранного фильтра или пустая строка
     */
    QString showFilterSelectionDialog();

    FilterChainModel* filterChainModel_;    ///< Модель цепочки фильтров
    QVBoxLayout* filterListLayout_;       ///< Layout для списка фильтров
    std::vector<FilterItemWidget*> filterItemWidgets_;  ///< Виджеты элементов фильтров
    Ui::FilterChainPanel* ui_;            ///< Указатель на UI объект
};

