#include <gui/filters/PosterizeFilterWidget.h>

#include <QLabel>
#include <QSlider>

#include "ui_PosterizeFilterWidget.h"

namespace
{
    constexpr int DefaultPosterizeLevels = 4;
}

PosterizeFilterWidget::PosterizeFilterWidget(QWidget* parent)
    : BaseFilterConfigWidget(parent)
    , ui_(new Ui::PosterizeFilterWidget())
    , updating_(false)
{
    ui_->setupUi(this);
    ui_->levelsSlider->setValue(DefaultPosterizeLevels);
    updateValueLabel(DefaultPosterizeLevels);
    initializeConnections();
}

void PosterizeFilterWidget::setParameters(const std::map<std::string, QVariant>& parameters)
{
    const auto iterator = parameters.find("posterize_levels");
    const int levels = iterator != parameters.end()
        ? iterator->second.toInt()
        : DefaultPosterizeLevels;

    updating_ = true;
    ui_->levelsSlider->setValue(levels);
    updateValueLabel(levels);
    updating_ = false;
}

std::map<std::string, QVariant> PosterizeFilterWidget::getParameters() const
{
    std::map<std::string, QVariant> result;
    result.emplace("posterize_levels", QVariant(ui_->levelsSlider->value()));
    return result;
}

void PosterizeFilterWidget::onLevelsChanged(int value)
{
    if (updating_)
    {
        return;
    }

    updateValueLabel(value);
    emit parameterChanged("posterize_levels", QVariant(value));
}

void PosterizeFilterWidget::initializeConnections()
{
    QObject::connect(
        ui_->levelsSlider,
        &QSlider::valueChanged,
        this,
        &PosterizeFilterWidget::onLevelsChanged);
}

void PosterizeFilterWidget::updateValueLabel(int value)
{
    ui_->valueLabel->setText(QString::number(value));
}

