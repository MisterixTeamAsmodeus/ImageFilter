#pragma once

#include <gui/BaseFilterConfigWidget.h>

namespace Ui
{
    class BoxBlurFilterWidget;
}

/**
 * @brief Виджет настройки фильтра box blur.
 */
class BoxBlurFilterWidget : public BaseFilterConfigWidget
{
    Q_OBJECT

public:
    explicit BoxBlurFilterWidget(QWidget* parent = nullptr);

    void setParameters(const std::map<std::string, QVariant>& parameters) override;
    std::map<std::string, QVariant> getParameters() const override;

private slots:
    void onRadiusChanged(int value);

private:
    Ui::BoxBlurFilterWidget* ui_;
    bool updating_;
};

