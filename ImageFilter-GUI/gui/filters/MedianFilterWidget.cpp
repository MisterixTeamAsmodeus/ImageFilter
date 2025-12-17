#include <gui/filters/MedianFilterWidget.h>

#include <QLabel>
#include <QSlider>

#include "ui_MedianFilterWidget.h"

namespace
{
    constexpr int DefaultMedianRadius = 2;
}

MedianFilterWidget::MedianFilterWidget(QWidget* parent)
    : BaseFilterConfigWidget(parent)
    , ui_(new Ui::MedianFilterWidget())
    , updating_(false)
{
    ui_->setupUi(this);
    ui_->radiusSlider->setValue(DefaultMedianRadius);
    updateValueLabel(DefaultMedianRadius);
    initializeConnections();
}

void MedianFilterWidget::setParameters(const std::map<std::string, QVariant>& parameters)
{
    const auto iterator = parameters.find("median_radius");
    const int radius = iterator != parameters.end()
        ? iterator->second.toInt()
        : DefaultMedianRadius;

    updating_ = true;
    ui_->radiusSlider->setValue(radius);
    updateValueLabel(radius);
    updating_ = false;
}

std::map<std::string, QVariant> MedianFilterWidget::getParameters() const
{
    std::map<std::string, QVariant> result;
    result.emplace("median_radius", QVariant(ui_->radiusSlider->value()));
    return result;
}

void MedianFilterWidget::onRadiusChanged(int value)
{
    if (updating_)
    {
        return;
    }

    updateValueLabel(value);
    emit parameterChanged("median_radius", QVariant(value));
}

void MedianFilterWidget::initializeConnections()
{
    QObject::connect(
        ui_->radiusSlider,
        &QSlider::valueChanged,
        this,
        &MedianFilterWidget::onRadiusChanged);
}

void MedianFilterWidget::updateValueLabel(int value)
{
    ui_->valueLabel->setText(QString::number(value));
}

