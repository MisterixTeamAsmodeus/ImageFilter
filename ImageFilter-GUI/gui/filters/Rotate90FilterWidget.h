#pragma once

#include <gui/BaseFilterConfigWidget.h>

namespace Ui
{
    class Rotate90FilterWidget;
}

/**
 * @brief Виджет настройки фильтра поворота на 90 градусов.
 *
 * Отвечает за отображение и редактирование параметра
 * counter_clockwise, используя UI, описанный в Rotate90FilterWidget.ui.
 */
class Rotate90FilterWidget : public BaseFilterConfigWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор виджета поворота.
     * @param parent Родительский виджет.
     */
    explicit Rotate90FilterWidget(QWidget* parent = nullptr);

    /**
     * @brief Устанавливает значения параметров фильтра.
     * @param parameters Карта параметров по имени.
     */
    void setParameters(const std::map<std::string, QVariant>& parameters) override;

    /**
     * @brief Возвращает текущие значения параметров фильтра.
     * @return Карта параметров по имени.
     */
    std::map<std::string, QVariant> getParameters() const override;

private slots:
    /**
     * @brief Обработчик изменения направления поворота.
     * @param checked Состояние чекбокса (true - против часовой стрелки).
     */
    void onCounterClockwiseChanged(bool checked);

private:
    Ui::Rotate90FilterWidget* ui_; ///< Сгенерированный Qt UI.
};

