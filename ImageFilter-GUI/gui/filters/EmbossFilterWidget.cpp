#include <gui/filters/EmbossFilterWidget.h>

#include <QLabel>
#include <QSlider>

#include "ui_EmbossFilterWidget.h"

namespace
{
    constexpr double DefaultEmbossStrength = 1.0;
}

EmbossFilterWidget::EmbossFilterWidget(QWidget* parent)
    : BaseFilterConfigWidget(parent)
    , ui_(new Ui::EmbossFilterWidget())
    , updating_(false)
{
    ui_->setupUi(this);
    const int sliderValue = static_cast<int>(DefaultEmbossStrength * 10);
    ui_->strengthSlider->setValue(sliderValue);
    updateValueLabel(DefaultEmbossStrength);
    initializeConnections();
}

void EmbossFilterWidget::setParameters(const std::map<std::string, QVariant>& parameters)
{
    const auto iterator = parameters.find("emboss_strength");
    const double strength = iterator != parameters.end()
        ? iterator->second.toDouble()
        : DefaultEmbossStrength;

    updating_ = true;
    const int sliderValue = static_cast<int>(strength * 10);
    ui_->strengthSlider->setValue(sliderValue);
    updateValueLabel(strength);
    updating_ = false;
}

std::map<std::string, QVariant> EmbossFilterWidget::getParameters() const
{
    std::map<std::string, QVariant> result;
    const double strength = static_cast<double>(ui_->strengthSlider->value()) / 10.0;
    result.emplace("emboss_strength", QVariant(strength));
    return result;
}

void EmbossFilterWidget::onStrengthChanged(int value)
{
    if (updating_)
    {
        return;
    }

    const double strength = static_cast<double>(value) / 10.0;
    updateValueLabel(strength);
    emit parameterChanged("emboss_strength", QVariant(strength));
}

void EmbossFilterWidget::initializeConnections()
{
    QObject::connect(
        ui_->strengthSlider,
        &QSlider::valueChanged,
        this,
        &EmbossFilterWidget::onStrengthChanged);
}

void EmbossFilterWidget::updateValueLabel(double value)
{
    ui_->valueLabel->setText(QString::number(value, 'f', 1));
}

