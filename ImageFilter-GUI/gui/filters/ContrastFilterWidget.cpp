#include <gui/filters/ContrastFilterWidget.h>

#include <QDoubleSpinBox>

#include "ui_ContrastFilterWidget.h"

namespace
{
    constexpr double DefaultContrastFactor = 1.5;
}

ContrastFilterWidget::ContrastFilterWidget(QWidget* parent)
    : BaseFilterConfigWidget(parent)
    , ui_(new Ui::ContrastFilterWidget())
    , updating_(false)
{
    ui_->setupUi(this);
    ui_->contrastSpinBox->setValue(DefaultContrastFactor);

    QObject::connect(
        ui_->contrastSpinBox,
        QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this,
        &ContrastFilterWidget::onContrastChanged);
}

void ContrastFilterWidget::setParameters(const std::map<std::string, QVariant>& parameters)
{
    const auto iterator = parameters.find("contrast_factor");
    const double factor = iterator != parameters.end()
        ? iterator->second.toDouble()
        : DefaultContrastFactor;

    updating_ = true;
    ui_->contrastSpinBox->setValue(factor);
    updating_ = false;
}

std::map<std::string, QVariant> ContrastFilterWidget::getParameters() const
{
    std::map<std::string, QVariant> result;
    result.emplace("contrast_factor", QVariant(ui_->contrastSpinBox->value()));
    return result;
}

void ContrastFilterWidget::onContrastChanged(double value)
{
    if (updating_)
    {
        return;
    }

    emit parameterChanged("contrast_factor", QVariant(value));
}

