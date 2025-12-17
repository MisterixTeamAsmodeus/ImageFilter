#pragma once

#include <QString>
#include <QVariant>
#include <QWidget>
#include <map>
#include <memory>
#include <string>

namespace Ui {
class FilterItemWidget;
}

// Forward declarations
class QLabel;
class QPushButton;
class QToolButton;
class QDoubleSpinBox;
class QSpinBox;
class QComboBox;
class QCheckBox;
class QVBoxLayout;
class QHBoxLayout;
class BaseFilterConfigWidget;

/**
 * @brief Виджет элемента фильтра в цепочке
 *
 * Отображает один фильтр с его параметрами и предоставляет интерфейс
 * для редактирования параметров, удаления и изменения порядка фильтра.
 */
class FilterItemWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Конструктор виджета элемента фильтра
     * @param filterIndex Индекс фильтра в цепочке
     * @param filterName Имя фильтра
     * @param parameters Параметры фильтра
     * @param canRemove Можно ли удалить фильтр (false если это единственный фильтр)
     * @param canMoveUp Можно ли переместить фильтр вверх
     * @param canMoveDown Можно ли переместить фильтр вниз
     * @param parent Родительский виджет
     */
    explicit FilterItemWidget(size_t filterIndex, const std::string& filterName,
                              const std::map<std::string, QVariant>& parameters, bool canRemove, bool canMoveUp,
                              bool canMoveDown, QWidget* parent = nullptr);

    /**
     * @brief Деструктор
     */
    ~FilterItemWidget() override;

    /**
     * @brief Обновляет параметры фильтра
     * @param parameters Новые параметры
     */
    void updateParameters(const std::map<std::string, QVariant>& parameters);

    /**
     * @brief Обновляет имя фильтра
     * @param filterName Новое имя фильтра
     */
    void updateFilterName(const std::string& filterName);

    /**
     * @brief Обновляет состояние кнопок управления
     * @param canRemove Можно ли удалить фильтр
     * @param canMoveUp Можно ли переместить фильтр вверх
     * @param canMoveDown Можно ли переместить фильтр вниз
     */
    void updateButtonsState(bool canRemove, bool canMoveUp, bool canMoveDown);

signals:
    /**
     * @brief Сигнал об изменении параметра фильтра
     * @param filterIndex Индекс фильтра
     * @param paramName Имя параметра
     * @param value Новое значение параметра
     */
    void parameterChanged(size_t filterIndex, const std::string& paramName, const QVariant& value);

    /**
     * @brief Сигнал о запросе удаления фильтра
     * @param filterIndex Индекс фильтра для удаления
     */
    void removeRequested(size_t filterIndex);

    /**
     * @brief Сигнал о запросе перемещения фильтра вверх
     * @param filterIndex Индекс фильтра для перемещения
     */
    void moveUpRequested(size_t filterIndex);

    /**
     * @brief Сигнал о запросе перемещения фильтра вниз
     * @param filterIndex Индекс фильтра для перемещения
     */
    void moveDownRequested(size_t filterIndex);

    /**
     * @brief Сигнал о запросе изменения типа фильтра
     * @param filterIndex Индекс фильтра для изменения типа
     */
    void filterTypeChangeRequested(size_t filterIndex);

private slots:
    /**
     * @brief Обработчик нажатия кнопки удаления
     */
    void onRemoveClicked();

    /**
     * @brief Обработчик нажатия кнопки "Вверх"
     */
    void onMoveUpClicked();

    /**
     * @brief Обработчик нажатия кнопки "Вниз"
     */
    void onMoveDownClicked();

    /**
     * @brief Обработчик нажатия кнопки изменения типа фильтра
     */
    void onChangeFilterTypeClicked();

private:
    /**
     * @brief Обработчик переключения видимости параметров фильтра.
     * @param checked Флаг, указывающий, должны ли параметры быть видимыми.
     */
    void onToggleParametersClicked(bool checked);

    /**
     * @brief Создает конфигурационный виджет для указанного фильтра
     *        и добавляет его в layout параметров.
     * @param filterName Имя фильтра
     * @param parameters Параметры фильтра
     */
    void createConfigWidget(const std::string& filterName, const std::map<std::string, QVariant>& parameters);

    /**
     * @brief Получает русское название фильтра
     * @param filterName Имя фильтра на английском
     * @return Русское название фильтра
     */
    static QString getFilterDisplayName(const std::string& filterName);

    size_t filterIndex_;                                   ///< Индекс фильтра в цепочке
    std::string filterName_;                               ///< Имя фильтра
    Ui::FilterItemWidget* ui_;                             ///< Автосгенерированный UI класс
    BaseFilterConfigWidget* configWidget_;                 ///< Конфигурационный виджет фильтра
    bool parametersVisible_ = true;                        ///< Текущее состояние видимости параметров
};
