#include <gui/filters/BoxBlurFilterWidget.h>

#include <QSpinBox>

#include "ui_BoxBlurFilterWidget.h"

namespace
{
    constexpr int DefaultBoxBlurRadius = 5;
}

BoxBlurFilterWidget::BoxBlurFilterWidget(QWidget* parent)
    : BaseFilterConfigWidget(parent)
    , ui_(new Ui::BoxBlurFilterWidget())
    , updating_(false)
{
    ui_->setupUi(this);
    ui_->radiusSpinBox->setValue(DefaultBoxBlurRadius);

    QObject::connect(
        ui_->radiusSpinBox,
        QOverload<int>::of(&QSpinBox::valueChanged),
        this,
        &BoxBlurFilterWidget::onRadiusChanged);
}

void BoxBlurFilterWidget::setParameters(const std::map<std::string, QVariant>& parameters)
{
    const auto iterator = parameters.find("box_blur_radius");
    const int radius = iterator != parameters.end()
        ? iterator->second.toInt()
        : DefaultBoxBlurRadius;

    updating_ = true;
    ui_->radiusSpinBox->setValue(radius);
    updating_ = false;
}

std::map<std::string, QVariant> BoxBlurFilterWidget::getParameters() const
{
    std::map<std::string, QVariant> result;
    result.emplace("box_blur_radius", QVariant(ui_->radiusSpinBox->value()));
    return result;
}

void BoxBlurFilterWidget::onRadiusChanged(int value)
{
    if (updating_)
    {
        return;
    }

    emit parameterChanged("box_blur_radius", QVariant(value));
}

