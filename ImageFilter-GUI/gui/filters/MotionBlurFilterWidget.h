#pragma once

#include <gui/BaseFilterConfigWidget.h>

namespace Ui
{
    class MotionBlurFilterWidget;
}

/**
 * @brief Виджет настройки фильтра размытия движения.
 */
class MotionBlurFilterWidget : public BaseFilterConfigWidget
{
    Q_OBJECT

public:
    explicit MotionBlurFilterWidget(QWidget* parent = nullptr);

    void setParameters(const std::map<std::string, QVariant>& parameters) override;
    std::map<std::string, QVariant> getParameters() const override;

private slots:
    void onLengthChanged(int value);
    void onAngleChanged(double value);

private:
    Ui::MotionBlurFilterWidget* ui_;
    bool updating_;
};

