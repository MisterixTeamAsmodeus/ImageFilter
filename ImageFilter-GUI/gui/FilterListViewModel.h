#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVector>

/**
 * @brief Модель списка доступных фильтров для QML.
 *
 * Представляет фильтры в виде списка, пригодного для использования
 * в компонентах QML (`ListView`, `ComboBox` и т.п.).
 *
 * Модель содержит базовую информацию о фильтрах (идентификатор, имя, описание)
 * и предоставляет доступ к данным только через интерфейс QAbstractListModel.
 */
class FilterListViewModel : public QAbstractListModel
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор модели списка фильтров.
     * @param parent Родительский QObject.
     */
    explicit FilterListViewModel(QObject* parent = nullptr);

    /**
     * @brief Возвращает количество элементов в модели.
     * @param parent Родительский индекс (не используется).
     * @return Количество фильтров.
     */
    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /**
     * @brief Возвращает данные по роли для указанного индекса.
     * @param index Индекс элемента.
     * @param role Роль данных.
     * @return Значение в виде QVariant.
     */
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Возвращает отображение ролей модели.
     * @return Ассоциативный массив ролей.
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private:
    /**
     * @brief Структура с информацией о фильтре.
     */
    struct FilterInfo
    {
        QString id;
        QString name;
        QString description;
    };

    /**
     * @brief Внутреннее хранилище описаний фильтров.
     */
    QVector<FilterInfo> filters_;

    /**
     * @brief Инициализирует набор фильтров по умолчанию.
     */
    void initializeDefaultFilters();
};


