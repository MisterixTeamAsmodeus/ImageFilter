#include <gui/filters/BrightnessFilterWidget.h>

#include <QLabel>
#include <QSlider>

#include "ui_BrightnessFilterWidget.h"

namespace
{
    constexpr double DefaultBrightnessFactor = 1.2;
}

BrightnessFilterWidget::BrightnessFilterWidget(QWidget* parent)
    : BaseFilterConfigWidget(parent)
    , ui_(new Ui::BrightnessFilterWidget())
{
    ui_->setupUi(this);
    const int sliderValue = static_cast<int>(DefaultBrightnessFactor * 10);
    ui_->brightnessSlider->setValue(sliderValue);
    updateValueLabel(DefaultBrightnessFactor);
    initializeConnections();
}

void BrightnessFilterWidget::setParameters(const std::map<std::string, QVariant>& parameters)
{
    const auto iterator = parameters.find("brightness_factor");
    const double factor = iterator != parameters.end()
        ? iterator->second.toDouble()
        : DefaultBrightnessFactor;

    const int sliderValue = static_cast<int>(factor * 10);
    ui_->brightnessSlider->setValue(sliderValue);
    updateValueLabel(factor);
}

std::map<std::string, QVariant> BrightnessFilterWidget::getParameters() const
{
    std::map<std::string, QVariant> result;
    const double factor = static_cast<double>(ui_->brightnessSlider->value()) / 10.0;
    result.emplace("brightness_factor", QVariant(factor));
    return result;
}

void BrightnessFilterWidget::onBrightnessChanged(int value)
{
    const double factor = static_cast<double>(value) / 10.0;
    updateValueLabel(factor);
    emit parameterChanged("brightness_factor", QVariant(factor));
}

void BrightnessFilterWidget::initializeConnections()
{
    QObject::connect(
        ui_->brightnessSlider,
        &QSlider::valueChanged,
        this,
        &BrightnessFilterWidget::onBrightnessChanged);
}

void BrightnessFilterWidget::updateValueLabel(double value)
{
    ui_->valueLabel->setText(QString::number(value, 'f', 1) + "x");
}


