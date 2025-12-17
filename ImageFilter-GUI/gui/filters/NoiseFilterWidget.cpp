#include <gui/filters/NoiseFilterWidget.h>

#include <QLabel>
#include <QSlider>

#include "ui_NoiseFilterWidget.h"

namespace
{
    constexpr double DefaultNoiseIntensity = 0.1;
}

NoiseFilterWidget::NoiseFilterWidget(QWidget* parent)
    : BaseFilterConfigWidget(parent)
    , ui_(new Ui::NoiseFilterWidget())
    , updating_(false)
{
    ui_->setupUi(this);
    const int sliderValue = static_cast<int>(DefaultNoiseIntensity * 100);
    ui_->intensitySlider->setValue(sliderValue);
    updateValueLabel(DefaultNoiseIntensity);
    initializeConnections();
}

void NoiseFilterWidget::setParameters(const std::map<std::string, QVariant>& parameters)
{
    const auto iterator = parameters.find("noise_intensity");
    const double intensity = iterator != parameters.end()
        ? iterator->second.toDouble()
        : DefaultNoiseIntensity;

    updating_ = true;
    const int sliderValue = static_cast<int>(intensity * 100);
    ui_->intensitySlider->setValue(sliderValue);
    updateValueLabel(intensity);
    updating_ = false;
}

std::map<std::string, QVariant> NoiseFilterWidget::getParameters() const
{
    std::map<std::string, QVariant> result;
    const double intensity = static_cast<double>(ui_->intensitySlider->value()) / 100.0;
    result.emplace("noise_intensity", QVariant(intensity));
    return result;
}

void NoiseFilterWidget::onIntensityChanged(int value)
{
    if (updating_)
    {
        return;
    }

    const double intensity = static_cast<double>(value) / 100.0;
    updateValueLabel(intensity);
    emit parameterChanged("noise_intensity", QVariant(intensity));
}

void NoiseFilterWidget::initializeConnections()
{
    QObject::connect(
        ui_->intensitySlider,
        &QSlider::valueChanged,
        this,
        &NoiseFilterWidget::onIntensityChanged);
}

void NoiseFilterWidget::updateValueLabel(double value)
{
    ui_->valueLabel->setText(QString::number(value, 'f', 2));
}

