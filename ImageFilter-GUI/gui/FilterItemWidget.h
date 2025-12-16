#pragma once

#include <QWidget>
#include <QString>
#include <QVariant>
#include <map>
#include <string>

// Forward declarations
class QLabel;
class QPushButton;
class QDoubleSpinBox;
class QSpinBox;
class QComboBox;
class QCheckBox;
class QVBoxLayout;
class QHBoxLayout;
class FilterChainModel;

/**
 * @brief Виджет элемента фильтра в цепочке
 *
 * Отображает один фильтр с его параметрами и предоставляет интерфейс
 * для редактирования параметров, удаления и изменения порядка фильтра.
 */
class FilterItemWidget : public QWidget
{
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
     * @param filterChainModel Модель цепочки фильтров
     * @param parent Родительский виджет
     */
    explicit FilterItemWidget(
        size_t filterIndex,
        const std::string& filterName,
        const std::map<std::string, QVariant>& parameters,
        bool canRemove,
        bool canMoveUp,
        bool canMoveDown,
        FilterChainModel* filterChainModel,
        QWidget* parent = nullptr);

    /**
     * @brief Деструктор
     */
    ~FilterItemWidget() override = default;

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
     * @brief Обработчик изменения параметра
     */
    void onParameterChanged();

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
     * @brief Инициализирует UI виджета
     */
    void setupUI();

    /**
     * @brief Создает виджеты параметров для фильтра
     * @param filterName Имя фильтра
     * @param parameters Параметры фильтра
     */
    void createParameterWidgets(const std::string& filterName, const std::map<std::string, QVariant>& parameters);

    /**
     * @brief Создает виджет для параметра brightness_factor
     */
    void createBrightnessWidget(const QVariant& value);

    /**
     * @brief Создает виджет для параметра contrast_factor
     */
    void createContrastWidget(const QVariant& value);

    /**
     * @brief Создает виджет для параметра saturation_factor
     */
    void createSaturationWidget(const QVariant& value);

    /**
     * @brief Создает виджет для параметра blur_radius
     */
    void createBlurRadiusWidget(const QVariant& value);

    /**
     * @brief Создает виджет для параметра box_blur_radius
     */
    void createBoxBlurRadiusWidget(const QVariant& value);

    /**
     * @brief Создает виджет для параметра motion_blur_length
     */
    void createMotionBlurLengthWidget(const QVariant& value);

    /**
     * @brief Создает виджет для параметра motion_blur_angle
     */
    void createMotionBlurAngleWidget(const QVariant& value);

    /**
     * @brief Создает виджет для параметра median_radius
     */
    void createMedianRadiusWidget(const QVariant& value);

    /**
     * @brief Создает виджет для параметра noise_intensity
     */
    void createNoiseIntensityWidget(const QVariant& value);

    /**
     * @brief Создает виджет для параметра posterize_levels
     */
    void createPosterizeLevelsWidget(const QVariant& value);

    /**
     * @brief Создает виджет для параметра threshold_value
     */
    void createThresholdValueWidget(const QVariant& value);

    /**
     * @brief Создает виджет для параметра vignette_strength
     */
    void createVignetteStrengthWidget(const QVariant& value);

    /**
     * @brief Создает виджет для параметра sharpen_strength
     */
    void createSharpenStrengthWidget(const QVariant& value);

    /**
     * @brief Создает виджет для параметра edge_sensitivity
     */
    void createEdgeSensitivityWidget(const QVariant& value);

    /**
     * @brief Создает виджет для параметра edge_operator
     */
    void createEdgeOperatorWidget(const QVariant& value);

    /**
     * @brief Создает виджет для параметра emboss_strength
     */
    void createEmbossStrengthWidget(const QVariant& value);

    /**
     * @brief Создает виджет для параметра counter_clockwise
     */
    void createCounterClockwiseWidget(const QVariant& value);

    /**
     * @brief Получает русское название фильтра
     * @param filterName Имя фильтра на английском
     * @return Русское название фильтра
     */
    static QString getFilterDisplayName(const std::string& filterName);

    size_t filterIndex_;                          ///< Индекс фильтра в цепочке
    std::string filterName_;                     ///< Имя фильтра
    FilterChainModel* filterChainModel_;         ///< Модель цепочки фильтров
    QVBoxLayout* mainLayout_;                   ///< Основной layout
    QHBoxLayout* headerLayout_;                  ///< Layout заголовка
    QLabel* filterNameLabel_;                   ///< Метка с именем фильтра
    QPushButton* changeTypeButton_;             ///< Кнопка изменения типа фильтра
    QPushButton* removeButton_;                  ///< Кнопка удаления
    QPushButton* moveUpButton_;                 ///< Кнопка "Вверх"
    QPushButton* moveDownButton_;               ///< Кнопка "Вниз"
    QVBoxLayout* parametersLayout_;             ///< Layout для параметров
    std::map<std::string, QWidget*> parameterWidgets_;  ///< Виджеты параметров по имени
    bool updatingParameters_;                   ///< Флаг обновления параметров (для предотвращения рекурсии)
};

