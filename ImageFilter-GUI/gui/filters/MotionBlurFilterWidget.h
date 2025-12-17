#pragma once

#include <gui/BaseFilterConfigWidget.h>

namespace Ui
{
    class MotionBlurFilterWidget;
}

/**
 * @brief Виджет настройки фильтра размытия движения.
 */
class MotionBlurFilterWidget : public BaseFilterConfigWidget
{
    Q_OBJECT

public:
    explicit MotionBlurFilterWidget(QWidget* parent = nullptr);

    void setParameters(const std::map<std::string, QVariant>& parameters) override;
    std::map<std::string, QVariant> getParameters() const override;

private slots:
    void onLengthChanged(int value);
    void onAngleChanged(int value);

private:
    /**
     * @brief Обновляет текст метки значения длины.
     * @param value Значение длины.
     */
    void updateLengthValueLabel(int value);

    /**
     * @brief Обновляет текст метки значения угла.
     * @param value Значение угла.
     */
    void updateAngleValueLabel(double value);

    Ui::MotionBlurFilterWidget* ui_;
    bool updating_;
};

