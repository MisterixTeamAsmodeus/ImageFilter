#include <gui/filters/VignetteFilterWidget.h>

#include <QLabel>
#include <QSlider>

#include "ui_VignetteFilterWidget.h"

namespace
{
    constexpr double DefaultVignetteStrength = 0.5;
}

VignetteFilterWidget::VignetteFilterWidget(QWidget* parent)
    : BaseFilterConfigWidget(parent)
    , ui_(new Ui::VignetteFilterWidget())
    , updating_(false)
{
    ui_->setupUi(this);
    const int sliderValue = static_cast<int>(DefaultVignetteStrength * 100);
    ui_->strengthSlider->setValue(sliderValue);
    updateValueLabel(DefaultVignetteStrength);
    initializeConnections();
}

void VignetteFilterWidget::setParameters(const std::map<std::string, QVariant>& parameters)
{
    const auto iterator = parameters.find("vignette_strength");
    const double strength = iterator != parameters.end()
        ? iterator->second.toDouble()
        : DefaultVignetteStrength;

    updating_ = true;
    const int sliderValue = static_cast<int>(strength * 100);
    ui_->strengthSlider->setValue(sliderValue);
    updateValueLabel(strength);
    updating_ = false;
}

std::map<std::string, QVariant> VignetteFilterWidget::getParameters() const
{
    std::map<std::string, QVariant> result;
    const double strength = static_cast<double>(ui_->strengthSlider->value()) / 100.0;
    result.emplace("vignette_strength", QVariant(strength));
    return result;
}

void VignetteFilterWidget::onStrengthChanged(int value)
{
    if (updating_)
    {
        return;
    }

    const double strength = static_cast<double>(value) / 100.0;
    updateValueLabel(strength);
    emit parameterChanged("vignette_strength", QVariant(strength));
}

void VignetteFilterWidget::initializeConnections()
{
    QObject::connect(
        ui_->strengthSlider,
        &QSlider::valueChanged,
        this,
        &VignetteFilterWidget::onStrengthChanged);
}

void VignetteFilterWidget::updateValueLabel(double value)
{
    ui_->valueLabel->setText(QString::number(value, 'f', 2));
}

