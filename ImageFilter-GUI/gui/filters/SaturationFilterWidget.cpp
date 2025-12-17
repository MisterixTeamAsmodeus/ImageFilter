#include <gui/filters/SaturationFilterWidget.h>

#include <QDoubleSpinBox>

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
    ui_->saturationSpinBox->setValue(DefaultSaturationFactor);

    QObject::connect(
        ui_->saturationSpinBox,
        QOverload<double>::of(&QDoubleSpinBox::valueChanged),
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
    ui_->saturationSpinBox->setValue(factor);
    updating_ = false;
}

std::map<std::string, QVariant> SaturationFilterWidget::getParameters() const
{
    std::map<std::string, QVariant> result;
    result.emplace("saturation_factor", QVariant(ui_->saturationSpinBox->value()));
    return result;
}

void SaturationFilterWidget::onSaturationChanged(double value)
{
    if (updating_)
    {
        return;
    }

    emit parameterChanged("saturation_factor", QVariant(value));
}

