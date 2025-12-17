#include <gui/filters/ThresholdFilterWidget.h>

#include <QLabel>
#include <QSlider>

#include "ui_ThresholdFilterWidget.h"

namespace
{
    constexpr int DefaultThresholdValue = 128;
}

ThresholdFilterWidget::ThresholdFilterWidget(QWidget* parent)
    : BaseFilterConfigWidget(parent)
    , ui_(new Ui::ThresholdFilterWidget())
    , updating_(false)
{
    ui_->setupUi(this);
    ui_->thresholdSlider->setValue(DefaultThresholdValue);
    updateValueLabel(DefaultThresholdValue);
    initializeConnections();
}

void ThresholdFilterWidget::setParameters(const std::map<std::string, QVariant>& parameters)
{
    const auto iterator = parameters.find("threshold_value");
    const int threshold = iterator != parameters.end()
        ? iterator->second.toInt()
        : DefaultThresholdValue;

    updating_ = true;
    ui_->thresholdSlider->setValue(threshold);
    updateValueLabel(threshold);
    updating_ = false;
}

std::map<std::string, QVariant> ThresholdFilterWidget::getParameters() const
{
    std::map<std::string, QVariant> result;
    result.emplace("threshold_value", QVariant(ui_->thresholdSlider->value()));
    return result;
}

void ThresholdFilterWidget::onThresholdChanged(int value)
{
    if (updating_)
    {
        return;
    }

    updateValueLabel(value);
    emit parameterChanged("threshold_value", QVariant(value));
}

void ThresholdFilterWidget::initializeConnections()
{
    QObject::connect(
        ui_->thresholdSlider,
        &QSlider::valueChanged,
        this,
        &ThresholdFilterWidget::onThresholdChanged);
}

void ThresholdFilterWidget::updateValueLabel(int value)
{
    ui_->valueLabel->setText(QString::number(value));
}

