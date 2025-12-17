#include <gui/filters/BlurFilterWidget.h>

#include <QDoubleSpinBox>

#include "ui_BlurFilterWidget.h"

namespace
{
    constexpr double DefaultBlurRadius = 5.0;
}

BlurFilterWidget::BlurFilterWidget(QWidget* parent)
    : BaseFilterConfigWidget(parent)
    , ui_(new Ui::BlurFilterWidget())
    , updating_(false)
{
    ui_->setupUi(this);
    ui_->radiusSpinBox->setValue(DefaultBlurRadius);

    QObject::connect(
        ui_->radiusSpinBox,
        QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this,
        &BlurFilterWidget::onRadiusChanged);
}

void BlurFilterWidget::setParameters(const std::map<std::string, QVariant>& parameters)
{
    const auto iterator = parameters.find("blur_radius");
    const double radius = iterator != parameters.end()
        ? iterator->second.toDouble()
        : DefaultBlurRadius;

    updating_ = true;
    ui_->radiusSpinBox->setValue(radius);
    updating_ = false;
}

std::map<std::string, QVariant> BlurFilterWidget::getParameters() const
{
    std::map<std::string, QVariant> result;
    result.emplace("blur_radius", QVariant(ui_->radiusSpinBox->value()));
    return result;
}

void BlurFilterWidget::onRadiusChanged(double value)
{
    if (updating_)
    {
        return;
    }

    emit parameterChanged("blur_radius", QVariant(value));
}

