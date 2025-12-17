#pragma once

#include <gui/BaseFilterConfigWidget.h>

namespace Ui
{
    class ContrastFilterWidget;
}

/**
 * @brief Виджет настройки фильтра контрастности.
 */
class ContrastFilterWidget : public BaseFilterConfigWidget
{
    Q_OBJECT

public:
    explicit ContrastFilterWidget(QWidget* parent = nullptr);

    void setParameters(const std::map<std::string, QVariant>& parameters) override;
    std::map<std::string, QVariant> getParameters() const override;

private slots:
    void onContrastChanged(int value);

private:
    /**
     * @brief Обновляет текст метки значения.
     * @param value Значение коэффициента контрастности.
     */
    void updateValueLabel(double value);

    Ui::ContrastFilterWidget* ui_;
    bool updating_;
};

