#include <gui/filters/MotionBlurFilterWidget.h>

#include <QLabel>
#include <QSlider>

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
    ui_->lengthSlider->setValue(DefaultMotionBlurLength);
    updateLengthValueLabel(DefaultMotionBlurLength);
    ui_->angleSlider->setValue(static_cast<int>(DefaultMotionBlurAngle));
    updateAngleValueLabel(DefaultMotionBlurAngle);

    QObject::connect(
        ui_->lengthSlider,
        &QSlider::valueChanged,
        this,
        &MotionBlurFilterWidget::onLengthChanged);

    QObject::connect(
        ui_->angleSlider,
        &QSlider::valueChanged,
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
    ui_->lengthSlider->setValue(length);
    updateLengthValueLabel(length);
    ui_->angleSlider->setValue(static_cast<int>(angle));
    updateAngleValueLabel(angle);
    updating_ = false;
}

std::map<std::string, QVariant> MotionBlurFilterWidget::getParameters() const
{
    std::map<std::string, QVariant> result;
    result.emplace("motion_blur_length", QVariant(ui_->lengthSlider->value()));
    result.emplace("motion_blur_angle", QVariant(static_cast<double>(ui_->angleSlider->value())));
    return result;
}

void MotionBlurFilterWidget::onLengthChanged(int value)
{
    if (updating_)
    {
        return;
    }

    updateLengthValueLabel(value);
    emit parameterChanged("motion_blur_length", QVariant(value));
}

void MotionBlurFilterWidget::onAngleChanged(int value)
{
    if (updating_)
    {
        return;
    }

    const double angle = static_cast<double>(value);
    updateAngleValueLabel(angle);
    emit parameterChanged("motion_blur_angle", QVariant(angle));
}

void MotionBlurFilterWidget::updateLengthValueLabel(int value)
{
    ui_->lengthValueLabel->setText(QString::number(value));
}

void MotionBlurFilterWidget::updateAngleValueLabel(double value)
{
    ui_->angleValueLabel->setText(QString::number(value, 'f', 1) + "°");
}


