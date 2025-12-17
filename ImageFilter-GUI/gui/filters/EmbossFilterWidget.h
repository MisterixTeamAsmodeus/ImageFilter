#pragma once

#include <gui/BaseFilterConfigWidget.h>

namespace Ui
{
    class EmbossFilterWidget;
}

/**
 * @brief Виджет настройки фильтра рельефа.
 *
 * Отвечает за отображение и редактирование параметра
 * emboss_strength, используя UI, описанный в EmbossFilterWidget.ui.
 */
class EmbossFilterWidget : public BaseFilterConfigWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор виджета рельефа.
     * @param parent Родительский виджет.
     */
    explicit EmbossFilterWidget(QWidget* parent = nullptr);

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
     * @brief Обработчик изменения силы рельефа.
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
     * @param value Значение силы рельефа.
     */
    void updateValueLabel(double value);

    Ui::EmbossFilterWidget* ui_; ///< Сгенерированный Qt UI.
    bool updating_; ///< Флаг для предотвращения циклических сигналов.
};

