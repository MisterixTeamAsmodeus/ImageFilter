#pragma once

#include <QDialog>

// Forward declarations
class QTreeView;
class QStandardItem;
class QLineEdit;
class FilterProxyModel;
class FilterListModel;

namespace Ui
{
    class FilterSelectionDialog;
}

/**
 * @brief Диалог выбора фильтра
 *
 * Предоставляет интерфейс для выбора фильтра из списка доступных фильтров,
 * сгруппированных по категориям. Поддерживает поиск по названию и сворачивание групп.
 */
class FilterSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор диалога выбора фильтра
     * @param parent Родительский виджет
     */
    explicit FilterSelectionDialog(QWidget* parent = nullptr);

    /**
     * @brief Деструктор
     */
    ~FilterSelectionDialog() override;

    /**
     * @brief Получает имя выбранного фильтра
     * @return Имя выбранного фильтра или пустая строка
     */
    QString getSelectedFilter() const;

private slots:
    /**
     * @brief Обработчик изменения текста поиска
     * @param text Текст для поиска
     */
    void onSearchTextChanged(const QString& text);

private:
    Ui::FilterSelectionDialog* ui_;      ///< Указатель на UI объект
    FilterListModel* model_;            ///< Модель данных для дерева фильтров
    FilterProxyModel* proxyModel_;      ///< Proxy модель для фильтрации
};

