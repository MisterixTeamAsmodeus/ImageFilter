#pragma once

#include <gui/BaseFilterConfigWidget.h>

namespace Ui
{
    class NoiseFilterWidget;
}

/**
 * @brief Виджет настройки фильтра шума.
 *
 * Отвечает за отображение и редактирование параметра
 * noise_intensity, используя UI, описанный в NoiseFilterWidget.ui.
 */
class NoiseFilterWidget : public BaseFilterConfigWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор виджета шума.
     * @param parent Родительский виджет.
     */
    explicit NoiseFilterWidget(QWidget* parent = nullptr);

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
     * @brief Обработчик изменения интенсивности шума.
     * @param value Новое значение слайдера (0-100, соответствует 0.0-1.0).
     */
    void onIntensityChanged(int value);

private:
    /**
     * @brief Инициализирует связи сигналов и слотов после setupUi.
     */
    void initializeConnections();

    /**
     * @brief Обновляет текст метки значения.
     * @param value Значение интенсивности шума.
     */
    void updateValueLabel(double value);

    Ui::NoiseFilterWidget* ui_; ///< Сгенерированный Qt UI.
    bool updating_; ///< Флаг для предотвращения циклических сигналов.
};

