#pragma once

#include <gui/BaseFilterConfigWidget.h>

namespace Ui
{
    class VignetteFilterWidget;
}

/**
 * @brief Виджет настройки фильтра виньетирования.
 *
 * Отвечает за отображение и редактирование параметра
 * vignette_strength, используя UI, описанный в VignetteFilterWidget.ui.
 */
class VignetteFilterWidget : public BaseFilterConfigWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор виджета виньетирования.
     * @param parent Родительский виджет.
     */
    explicit VignetteFilterWidget(QWidget* parent = nullptr);

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
     * @brief Обработчик изменения силы виньетирования.
     * @param value Новое значение слайдера (0-100, соответствует 0.0-1.0).
     */
    void onStrengthChanged(int value);

private:
    /**
     * @brief Инициализирует связи сигналов и слотов после setupUi.
     */
    void initializeConnections();

    /**
     * @brief Обновляет текст метки значения.
     * @param value Значение силы виньетирования.
     */
    void updateValueLabel(double value);

    Ui::VignetteFilterWidget* ui_; ///< Сгенерированный Qt UI.
    bool updating_; ///< Флаг для предотвращения циклических сигналов.
};

