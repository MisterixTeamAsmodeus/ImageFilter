#pragma once

#include <gui/BaseFilterConfigWidget.h>

namespace Ui
{
    class ThresholdFilterWidget;
}

/**
 * @brief Виджет настройки фильтра пороговой бинаризации.
 *
 * Отвечает за отображение и редактирование параметра
 * threshold_value, используя UI, описанный в ThresholdFilterWidget.ui.
 */
class ThresholdFilterWidget : public BaseFilterConfigWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор виджета пороговой бинаризации.
     * @param parent Родительский виджет.
     */
    explicit ThresholdFilterWidget(QWidget* parent = nullptr);

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
     * @brief Обработчик изменения порогового значения.
     * @param value Новое значение слайдера (0-255).
     */
    void onThresholdChanged(int value);

private:
    /**
     * @brief Инициализирует связи сигналов и слотов после setupUi.
     */
    void initializeConnections();

    /**
     * @brief Обновляет текст метки значения.
     * @param value Значение порога.
     */
    void updateValueLabel(int value);

    Ui::ThresholdFilterWidget* ui_; ///< Сгенерированный Qt UI.
    bool updating_; ///< Флаг для предотвращения циклических сигналов.
};

