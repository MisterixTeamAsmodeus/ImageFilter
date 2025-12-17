#include <gui/filters/SharpenFilterWidget.h>

#include <QLabel>
#include <QSlider>

#include "ui_SharpenFilterWidget.h"

namespace
{
    constexpr double DefaultSharpenStrength = 1.0;
}

SharpenFilterWidget::SharpenFilterWidget(QWidget* parent)
    : BaseFilterConfigWidget(parent)
    , ui_(new Ui::SharpenFilterWidget())
    , updating_(false)
{
    ui_->setupUi(this);
    const int sliderValue = static_cast<int>(DefaultSharpenStrength * 10);
    ui_->strengthSlider->setValue(sliderValue);
    updateValueLabel(DefaultSharpenStrength);
    initializeConnections();
}

void SharpenFilterWidget::setParameters(const std::map<std::string, QVariant>& parameters)
{
    const auto iterator = parameters.find("sharpen_strength");
    const double strength = iterator != parameters.end()
        ? iterator->second.toDouble()
        : DefaultSharpenStrength;

    updating_ = true;
    const int sliderValue = static_cast<int>(strength * 10);
    ui_->strengthSlider->setValue(sliderValue);
    updateValueLabel(strength);
    updating_ = false;
}

std::map<std::string, QVariant> SharpenFilterWidget::getParameters() const
{
    std::map<std::string, QVariant> result;
    const double strength = static_cast<double>(ui_->strengthSlider->value()) / 10.0;
    result.emplace("sharpen_strength", QVariant(strength));
    return result;
}

void SharpenFilterWidget::onStrengthChanged(int value)
{
    if (updating_)
    {
        return;
    }

    const double strength = static_cast<double>(value) / 10.0;
    updateValueLabel(strength);
    emit parameterChanged("sharpen_strength", QVariant(strength));
}

void SharpenFilterWidget::initializeConnections()
{
    QObject::connect(
        ui_->strengthSlider,
        &QSlider::valueChanged,
        this,
        &SharpenFilterWidget::onStrengthChanged);
}

void SharpenFilterWidget::updateValueLabel(double value)
{
    ui_->valueLabel->setText(QString::number(value, 'f', 1));
}

