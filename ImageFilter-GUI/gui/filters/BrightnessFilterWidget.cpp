#include <gui/filters/BrightnessFilterWidget.h>

#include <QDoubleSpinBox>

#include "ui_BrightnessFilterWidget.h"

namespace
{
    constexpr double DefaultBrightnessFactor = 1.2;
}

BrightnessFilterWidget::BrightnessFilterWidget(QWidget* parent)
    : BaseFilterConfigWidget(parent)
    , ui_(new Ui::BrightnessFilterWidget())
    , updating_(false)
{
    ui_->setupUi(this);
    ui_->brightnessSpinBox->setValue(DefaultBrightnessFactor);
    initializeConnections();
}

void BrightnessFilterWidget::setParameters(const std::map<std::string, QVariant>& parameters)
{
    const auto iterator = parameters.find("brightness_factor");
    const double factor = iterator != parameters.end()
        ? iterator->second.toDouble()
        : DefaultBrightnessFactor;

    updating_ = true;
    ui_->brightnessSpinBox->setValue(factor);
    updating_ = false;
}

std::map<std::string, QVariant> BrightnessFilterWidget::getParameters() const
{
    std::map<std::string, QVariant> result;
    result.emplace("brightness_factor", QVariant(ui_->brightnessSpinBox->value()));
    return result;
}

void BrightnessFilterWidget::onBrightnessChanged(double value)
{
    if (updating_)
    {
        return;
    }

    emit parameterChanged("brightness_factor", QVariant(value));
}

void BrightnessFilterWidget::initializeConnections()
{
    QObject::connect(
        ui_->brightnessSpinBox,
        QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this,
        &BrightnessFilterWidget::onBrightnessChanged);
}


