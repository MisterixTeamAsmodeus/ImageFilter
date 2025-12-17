#pragma once

#include <gui/BaseFilterConfigWidget.h>

namespace Ui
{
    class BrightnessFilterWidget;
}

/**
 * @brief Виджет настройки фильтра яркости.
 *
 * Отвечает только за отображение и редактирование параметра
 * brightness_factor, используя UI, описанный в BrightnessFilterWidget.ui.
 */
class BrightnessFilterWidget : public BaseFilterConfigWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор виджета яркости.
     * @param parent Родительский виджет.
     */
    explicit BrightnessFilterWidget(QWidget* parent = nullptr);

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
     * @brief Обработчик изменения значения яркости.
     * @param value Новое значение коэффициента яркости.
     */
    void onBrightnessChanged(double value);

private:
    /**
     * @brief Инициализирует связи сигналов и слотов после setupUi.
     */
    void initializeConnections();

    Ui::BrightnessFilterWidget* ui_; ///< Сгенерированный Qt UI.
    bool updating_;                  ///< Флаг предотвращения рекурсивных обновлений.
};


