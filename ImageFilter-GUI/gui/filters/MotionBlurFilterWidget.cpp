#include <gui/filters/MotionBlurFilterWidget.h>

#include <QDoubleSpinBox>
#include <QSpinBox>

#include "ui_MotionBlurFilterWidget.h"

namespace
{
    constexpr int DefaultMotionBlurLength = 10;
    constexpr double DefaultMotionBlurAngle = 0.0;
}

MotionBlurFilterWidget::MotionBlurFilterWidget(QWidget* parent)
    : BaseFilterConfigWidget(parent)
    , ui_(new Ui::MotionBlurFilterWidget())
    , updating_(false)
{
    ui_->setupUi(this);
    ui_->lengthSpinBox->setValue(DefaultMotionBlurLength);
    ui_->angleSpinBox->setValue(DefaultMotionBlurAngle);

    QObject::connect(
        ui_->lengthSpinBox,
        QOverload<int>::of(&QSpinBox::valueChanged),
        this,
        &MotionBlurFilterWidget::onLengthChanged);

    QObject::connect(
        ui_->angleSpinBox,
        QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this,
        &MotionBlurFilterWidget::onAngleChanged);
}

void MotionBlurFilterWidget::setParameters(const std::map<std::string, QVariant>& parameters)
{
    const auto lengthIterator = parameters.find("motion_blur_length");
    const int length = lengthIterator != parameters.end()
        ? lengthIterator->second.toInt()
        : DefaultMotionBlurLength;

    const auto angleIterator = parameters.find("motion_blur_angle");
    const double angle = angleIterator != parameters.end()
        ? angleIterator->second.toDouble()
        : DefaultMotionBlurAngle;

    updating_ = true;
    ui_->lengthSpinBox->setValue(length);
    ui_->angleSpinBox->setValue(angle);
    updating_ = false;
}

std::map<std::string, QVariant> MotionBlurFilterWidget::getParameters() const
{
    std::map<std::string, QVariant> result;
    result.emplace("motion_blur_length", QVariant(ui_->lengthSpinBox->value()));
    result.emplace("motion_blur_angle", QVariant(ui_->angleSpinBox->value()));
    return result;
}

void MotionBlurFilterWidget::onLengthChanged(int value)
{
    if (updating_)
    {
        return;
    }

    emit parameterChanged("motion_blur_length", QVariant(value));
}

void MotionBlurFilterWidget::onAngleChanged(double value)
{
    if (updating_)
    {
        return;
    }

    emit parameterChanged("motion_blur_angle", QVariant(value));
}

