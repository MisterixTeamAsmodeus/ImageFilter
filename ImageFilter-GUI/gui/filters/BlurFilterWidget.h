#pragma once

#include <gui/BaseFilterConfigWidget.h>

namespace Ui
{
    class BlurFilterWidget;
}

/**
 * @brief Виджет настройки фильтра размытия по Гауссу.
 */
class BlurFilterWidget : public BaseFilterConfigWidget
{
    Q_OBJECT

public:
    explicit BlurFilterWidget(QWidget* parent = nullptr);

    void setParameters(const std::map<std::string, QVariant>& parameters) override;
    std::map<std::string, QVariant> getParameters() const override;

private slots:
    void onRadiusChanged(int value);

private:
    /**
     * @brief Обновляет текст метки значения.
     * @param value Значение радиуса размытия.
     */
    void updateValueLabel(double value);

    Ui::BlurFilterWidget* ui_;
    bool updating_;
};

