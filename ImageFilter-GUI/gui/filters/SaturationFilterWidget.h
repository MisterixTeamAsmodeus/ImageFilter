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
    void onSaturationChanged(double value);

private:
    Ui::SaturationFilterWidget* ui_;
    bool updating_;
};

