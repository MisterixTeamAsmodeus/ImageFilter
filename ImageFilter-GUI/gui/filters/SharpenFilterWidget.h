#pragma once

#include <gui/BaseFilterConfigWidget.h>

namespace Ui
{
    class SharpenFilterWidget;
}

/**
 * @brief Виджет настройки фильтра повышения резкости.
 *
 * Отвечает за отображение и редактирование параметра
 * sharpen_strength, используя UI, описанный в SharpenFilterWidget.ui.
 */
class SharpenFilterWidget : public BaseFilterConfigWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор виджета резкости.
     * @param parent Родительский виджет.
     */
    explicit SharpenFilterWidget(QWidget* parent = nullptr);

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
     * @brief Обработчик изменения силы резкости.
     * @param value Новое значение слайдера (0-50, соответствует 0.0-5.0).
     */
    void onStrengthChanged(int value);

private:
    /**
     * @brief Инициализирует связи сигналов и слотов после setupUi.
     */
    void initializeConnections();

    /**
     * @brief Обновляет текст метки значения.
     * @param value Значение силы резкости.
     */
    void updateValueLabel(double value);

    Ui::SharpenFilterWidget* ui_; ///< Сгенерированный Qt UI.
    bool updating_; ///< Флаг для предотвращения циклических сигналов.
};

