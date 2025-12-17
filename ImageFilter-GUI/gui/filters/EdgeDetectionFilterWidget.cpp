#include <gui/filters/EdgeDetectionFilterWidget.h>

#include <QComboBox>
#include <QLabel>
#include <QSlider>

#include "ui_EdgeDetectionFilterWidget.h"

namespace
{
    constexpr double DefaultEdgeSensitivity = 0.5;
    constexpr const char* DefaultEdgeOperator = "sobel";
}

EdgeDetectionFilterWidget::EdgeDetectionFilterWidget(QWidget* parent)
    : BaseFilterConfigWidget(parent)
    , ui_(new Ui::EdgeDetectionFilterWidget())
    , updating_(false)
{
    ui_->setupUi(this);
    const int sliderValue = static_cast<int>(DefaultEdgeSensitivity * 100);
    ui_->sensitivitySlider->setValue(sliderValue);
    updateSensitivityLabel(DefaultEdgeSensitivity);
    ui_->operatorComboBox->setCurrentIndex(operatorStringToIndex(DefaultEdgeOperator));
    initializeConnections();
}

void EdgeDetectionFilterWidget::setParameters(const std::map<std::string, QVariant>& parameters)
{
    const auto sensitivityIterator = parameters.find("edge_sensitivity");
    const double sensitivity = sensitivityIterator != parameters.end()
        ? sensitivityIterator->second.toDouble()
        : DefaultEdgeSensitivity;

    const auto operatorIterator = parameters.find("edge_operator");
    const std::string operatorStr = operatorIterator != parameters.end()
        ? operatorIterator->second.toString().toStdString()
        : DefaultEdgeOperator;

    updating_ = true;
    const int sliderValue = static_cast<int>(sensitivity * 100);
    ui_->sensitivitySlider->setValue(sliderValue);
    updateSensitivityLabel(sensitivity);
    ui_->operatorComboBox->setCurrentIndex(operatorStringToIndex(operatorStr));
    updating_ = false;
}

std::map<std::string, QVariant> EdgeDetectionFilterWidget::getParameters() const
{
    std::map<std::string, QVariant> result;
    const double sensitivity = static_cast<double>(ui_->sensitivitySlider->value()) / 100.0;
    result.emplace("edge_sensitivity", QVariant(sensitivity));
    result.emplace("edge_operator", QVariant(QString::fromStdString(operatorIndexToString(ui_->operatorComboBox->currentIndex()))));
    return result;
}

void EdgeDetectionFilterWidget::onSensitivityChanged(int value)
{
    if (updating_)
    {
        return;
    }

    const double sensitivity = static_cast<double>(value) / 100.0;
    updateSensitivityLabel(sensitivity);
    emit parameterChanged("edge_sensitivity", QVariant(sensitivity));
}

void EdgeDetectionFilterWidget::onOperatorChanged(int index)
{
    if (updating_)
    {
        return;
    }

    const std::string operatorStr = operatorIndexToString(index);
    emit parameterChanged("edge_operator", QVariant(QString::fromStdString(operatorStr)));
}

void EdgeDetectionFilterWidget::initializeConnections()
{
    QObject::connect(
        ui_->sensitivitySlider,
        &QSlider::valueChanged,
        this,
        &EdgeDetectionFilterWidget::onSensitivityChanged);

    QObject::connect(
        ui_->operatorComboBox,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        &EdgeDetectionFilterWidget::onOperatorChanged);
}

void EdgeDetectionFilterWidget::updateSensitivityLabel(double value)
{
    ui_->sensitivityValueLabel->setText(QString::number(value, 'f', 2));
}

int EdgeDetectionFilterWidget::operatorStringToIndex(const std::string& operatorStr) const
{
    if (operatorStr == "prewitt")
    {
        return 1;
    }
    if (operatorStr == "scharr")
    {
        return 2;
    }
    return 0; // sobel по умолчанию
}

std::string EdgeDetectionFilterWidget::operatorIndexToString(int index) const
{
    switch (index)
    {
    case 1:
        return "prewitt";
    case 2:
        return "scharr";
    default:
        return "sobel";
    }
}

