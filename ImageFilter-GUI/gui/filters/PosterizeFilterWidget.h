#pragma once

#include <gui/BaseFilterConfigWidget.h>

namespace Ui
{
    class PosterizeFilterWidget;
}

/**
 * @brief Виджет настройки фильтра постеризации.
 *
 * Отвечает за отображение и редактирование параметра
 * posterize_levels, используя UI, описанный в PosterizeFilterWidget.ui.
 */
class PosterizeFilterWidget : public BaseFilterConfigWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор виджета постеризации.
     * @param parent Родительский виджет.
     */
    explicit PosterizeFilterWidget(QWidget* parent = nullptr);

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
     * @brief Обработчик изменения количества уровней.
     * @param value Новое значение слайдера (2-256).
     */
    void onLevelsChanged(int value);

private:
    /**
     * @brief Инициализирует связи сигналов и слотов после setupUi.
     */
    void initializeConnections();

    /**
     * @brief Обновляет текст метки значения.
     * @param value Значение количества уровней.
     */
    void updateValueLabel(int value);

    Ui::PosterizeFilterWidget* ui_; ///< Сгенерированный Qt UI.
    bool updating_; ///< Флаг для предотвращения циклических сигналов.
};

