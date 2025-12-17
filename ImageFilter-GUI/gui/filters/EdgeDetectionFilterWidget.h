#pragma once

#include <gui/BaseFilterConfigWidget.h>

namespace Ui
{
    class EdgeDetectionFilterWidget;
}

/**
 * @brief Виджет настройки фильтра детекции краёв.
 *
 * Отвечает за отображение и редактирование параметров
 * edge_sensitivity и edge_operator, используя UI, описанный в EdgeDetectionFilterWidget.ui.
 */
class EdgeDetectionFilterWidget : public BaseFilterConfigWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор виджета детекции краёв.
     * @param parent Родительский виджет.
     */
    explicit EdgeDetectionFilterWidget(QWidget* parent = nullptr);

    /**
     * @brief Устанавливает значения параметров фильтра.
     * @param parameters Карта параметров по имени.
     */
    void setParameters(const std::map<std::string, QVariant>& parameters) override;

    /**
     * @brief Возвращает текущие значения параметров фильтра.
     * @return Карта параметров по имени.
     */
    std::map<std::string, QVariant> getParameters() const override;

private slots:
    /**
     * @brief Обработчик изменения чувствительности.
     * @param value Новое значение слайдера (0-100, соответствует 0.0-1.0).
     */
    void onSensitivityChanged(int value);

    /**
     * @brief Обработчик изменения оператора.
     * @param index Индекс выбранного оператора в комбобоксе.
     */
    void onOperatorChanged(int index);

private:
    /**
     * @brief Инициализирует связи сигналов и слотов после setupUi.
     */
    void initializeConnections();

    /**
     * @brief Обновляет текст метки значения чувствительности.
     * @param value Значение чувствительности.
     */
    void updateSensitivityLabel(double value);

    /**
     * @brief Преобразует строку оператора в индекс комбобокса.
     * @param operatorStr Строка оператора ("sobel", "prewitt", "scharr").
     * @return Индекс в комбобоксе.
     */
    int operatorStringToIndex(const std::string& operatorStr) const;

    /**
     * @brief Преобразует индекс комбобокса в строку оператора.
     * @param index Индекс в комбобоксе.
     * @return Строка оператора.
     */
    std::string operatorIndexToString(int index) const;

    Ui::EdgeDetectionFilterWidget* ui_; ///< Сгенерированный Qt UI.
    bool updating_; ///< Флаг для предотвращения циклических сигналов.
};

