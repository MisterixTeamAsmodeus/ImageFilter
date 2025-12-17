#pragma once

#include <gui/BaseFilterConfigWidget.h>

namespace Ui
{
    class BlurFilterWidget;
}

/**
 * @brief Виджет настройки фильтра размытия по Гауссу.
 */
class BlurFilterWidget : public BaseFilterConfigWidget
{
    Q_OBJECT

public:
    explicit BlurFilterWidget(QWidget* parent = nullptr);

    void setParameters(const std::map<std::string, QVariant>& parameters) override;
    std::map<std::string, QVariant> getParameters() const override;

private slots:
    void onRadiusChanged(double value);

private:
    Ui::BlurFilterWidget* ui_;
    bool updating_;
};

