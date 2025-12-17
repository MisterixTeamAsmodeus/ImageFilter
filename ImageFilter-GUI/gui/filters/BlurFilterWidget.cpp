#include <gui/filters/BlurFilterWidget.h>

#include <QLabel>
#include <QSlider>

#include "ui_BlurFilterWidget.h"

namespace
{
    constexpr double DefaultBlurRadius = 5.0;
}

BlurFilterWidget::BlurFilterWidget(QWidget* parent)
    : BaseFilterConfigWidget(parent)
    , ui_(new Ui::BlurFilterWidget())
    , updating_(false)
{
    ui_->setupUi(this);
    const int sliderValue = static_cast<int>(DefaultBlurRadius * 10);
    ui_->radiusSlider->setValue(sliderValue);
    updateValueLabel(DefaultBlurRadius);

    QObject::connect(
        ui_->radiusSlider,
        &QSlider::valueChanged,
        this,
        &BlurFilterWidget::onRadiusChanged);
}

void BlurFilterWidget::setParameters(const std::map<std::string, QVariant>& parameters)
{
    const auto iterator = parameters.find("blur_radius");
    const double radius = iterator != parameters.end()
        ? iterator->second.toDouble()
        : DefaultBlurRadius;

    updating_ = true;
    const int sliderValue = static_cast<int>(radius * 10);
    ui_->radiusSlider->setValue(sliderValue);
    updateValueLabel(radius);
    updating_ = false;
}

std::map<std::string, QVariant> BlurFilterWidget::getParameters() const
{
    std::map<std::string, QVariant> result;
    const double radius = static_cast<double>(ui_->radiusSlider->value()) / 10.0;
    result.emplace("blur_radius", QVariant(radius));
    return result;
}

void BlurFilterWidget::onRadiusChanged(int value)
{
    if (updating_)
    {
        return;
    }

    const double radius = static_cast<double>(value) / 10.0;
    updateValueLabel(radius);
    emit parameterChanged("blur_radius", QVariant(radius));
}

void BlurFilterWidget::updateValueLabel(double value)
{
    ui_->valueLabel->setText(QString::number(value, 'f', 1));
}

