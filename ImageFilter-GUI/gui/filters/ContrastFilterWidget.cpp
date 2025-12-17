#include <gui/filters/ContrastFilterWidget.h>

#include <QLabel>
#include <QSlider>

#include "ui_ContrastFilterWidget.h"

namespace
{
    constexpr double DefaultContrastFactor = 1.5;
}

ContrastFilterWidget::ContrastFilterWidget(QWidget* parent)
    : BaseFilterConfigWidget(parent)
    , ui_(new Ui::ContrastFilterWidget())
    , updating_(false)
{
    ui_->setupUi(this);
    const int sliderValue = static_cast<int>(DefaultContrastFactor * 10);
    ui_->contrastSlider->setValue(sliderValue);
    updateValueLabel(DefaultContrastFactor);

    QObject::connect(
        ui_->contrastSlider,
        &QSlider::valueChanged,
        this,
        &ContrastFilterWidget::onContrastChanged);
}

void ContrastFilterWidget::setParameters(const std::map<std::string, QVariant>& parameters)
{
    const auto iterator = parameters.find("contrast_factor");
    const double factor = iterator != parameters.end()
        ? iterator->second.toDouble()
        : DefaultContrastFactor;

    updating_ = true;
    const int sliderValue = static_cast<int>(factor * 10);
    ui_->contrastSlider->setValue(sliderValue);
    updateValueLabel(factor);
    updating_ = false;
}

std::map<std::string, QVariant> ContrastFilterWidget::getParameters() const
{
    std::map<std::string, QVariant> result;
    const double factor = static_cast<double>(ui_->contrastSlider->value()) / 10.0;
    result.emplace("contrast_factor", QVariant(factor));
    return result;
}

void ContrastFilterWidget::onContrastChanged(int value)
{
    if (updating_)
    {
        return;
    }

    const double factor = static_cast<double>(value) / 10.0;
    updateValueLabel(factor);
    emit parameterChanged("contrast_factor", QVariant(factor));
}

void ContrastFilterWidget::updateValueLabel(double value)
{
    ui_->valueLabel->setText(QString::number(value, 'f', 1) + "x");
}

