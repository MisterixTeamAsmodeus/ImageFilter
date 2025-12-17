#include <gui/filters/SaturationFilterWidget.h>

#include <QLabel>
#include <QSlider>

#include "ui_SaturationFilterWidget.h"

namespace
{
    constexpr double DefaultSaturationFactor = 1.5;
}

SaturationFilterWidget::SaturationFilterWidget(QWidget* parent)
    : BaseFilterConfigWidget(parent)
    , ui_(new Ui::SaturationFilterWidget())
    , updating_(false)
{
    ui_->setupUi(this);
    const int sliderValue = static_cast<int>(DefaultSaturationFactor * 10);
    ui_->saturationSlider->setValue(sliderValue);
    updateValueLabel(DefaultSaturationFactor);

    QObject::connect(
        ui_->saturationSlider,
        &QSlider::valueChanged,
        this,
        &SaturationFilterWidget::onSaturationChanged);
}

void SaturationFilterWidget::setParameters(const std::map<std::string, QVariant>& parameters)
{
    const auto iterator = parameters.find("saturation_factor");
    const double factor = iterator != parameters.end()
        ? iterator->second.toDouble()
        : DefaultSaturationFactor;

    updating_ = true;
    const int sliderValue = static_cast<int>(factor * 10);
    ui_->saturationSlider->setValue(sliderValue);
    updateValueLabel(factor);
    updating_ = false;
}

std::map<std::string, QVariant> SaturationFilterWidget::getParameters() const
{
    std::map<std::string, QVariant> result;
    const double factor = static_cast<double>(ui_->saturationSlider->value()) / 10.0;
    result.emplace("saturation_factor", QVariant(factor));
    return result;
}

void SaturationFilterWidget::onSaturationChanged(int value)
{
    if (updating_)
    {
        return;
    }

    const double factor = static_cast<double>(value) / 10.0;
    updateValueLabel(factor);
    emit parameterChanged("saturation_factor", QVariant(factor));
}

void SaturationFilterWidget::updateValueLabel(double value)
{
    ui_->valueLabel->setText(QString::number(value, 'f', 1) + "x");
}

