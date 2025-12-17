#include <gui/filters/Rotate90FilterWidget.h>

#include <QCheckBox>

#include "ui_Rotate90FilterWidget.h"

namespace
{
    constexpr bool DefaultCounterClockwise = false;
}

Rotate90FilterWidget::Rotate90FilterWidget(QWidget* parent)
    : BaseFilterConfigWidget(parent)
    , ui_(new Ui::Rotate90FilterWidget())
{
    ui_->setupUi(this);
    ui_->counterClockwiseCheckBox->setChecked(DefaultCounterClockwise);

    QObject::connect(
        ui_->counterClockwiseCheckBox,
        &QCheckBox::toggled,
        this,
        &Rotate90FilterWidget::onCounterClockwiseChanged);
}

void Rotate90FilterWidget::setParameters(const std::map<std::string, QVariant>& parameters)
{
    const auto iterator = parameters.find("counter_clockwise");
    const bool counterClockwise = iterator != parameters.end()
        ? iterator->second.toBool()
        : DefaultCounterClockwise;

    ui_->counterClockwiseCheckBox->setChecked(counterClockwise);
}

std::map<std::string, QVariant> Rotate90FilterWidget::getParameters() const
{
    std::map<std::string, QVariant> result;
    result.emplace("counter_clockwise", QVariant(ui_->counterClockwiseCheckBox->isChecked()));
    return result;
}

void Rotate90FilterWidget::onCounterClockwiseChanged(bool checked)
{
    emit parameterChanged("counter_clockwise", QVariant(checked));
}

