#pragma once

#include <gui/BaseFilterConfigWidget.h>

namespace Ui
{
    class SaturationFilterWidget;
}

/**
 * @brief Виджет настройки фильтра насыщенности.
 */
class SaturationFilterWidget : public BaseFilterConfigWidget
{
    Q_OBJECT

public:
    explicit SaturationFilterWidget(QWidget* parent = nullptr);

    void setParameters(const std::map<std::string, QVariant>& parameters) override;
    std::map<std::string, QVariant> getParameters() const override;

private slots:
    void onSaturationChanged(int value);

private:
    /**
     * @brief Обновляет текст метки значения.
     * @param value Значение коэффициента насыщенности.
     */
    void updateValueLabel(double value);

    Ui::SaturationFilterWidget* ui_;
    bool updating_;
};

