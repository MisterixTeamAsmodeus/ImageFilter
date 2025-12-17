#pragma once

#include <gui/BaseFilterConfigWidget.h>

namespace Ui
{
    class MedianFilterWidget;
}

/**
 * @brief Виджет настройки медианного фильтра.
 *
 * Отвечает за отображение и редактирование параметра
 * median_radius, используя UI, описанный в MedianFilterWidget.ui.
 */
class MedianFilterWidget : public BaseFilterConfigWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор виджета медианного фильтра.
     * @param parent Родительский виджет.
     */
    explicit MedianFilterWidget(QWidget* parent = nullptr);

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
     * @brief Обработчик изменения радиуса.
     * @param value Новое значение слайдера (1-10).
     */
    void onRadiusChanged(int value);

private:
    /**
     * @brief Инициализирует связи сигналов и слотов после setupUi.
     */
    void initializeConnections();

    /**
     * @brief Обновляет текст метки значения.
     * @param value Значение радиуса.
     */
    void updateValueLabel(int value);

    Ui::MedianFilterWidget* ui_; ///< Сгенерированный Qt UI.
    bool updating_; ///< Флаг для предотвращения циклических сигналов.
};

