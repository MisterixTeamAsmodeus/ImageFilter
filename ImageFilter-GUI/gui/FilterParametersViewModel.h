#pragma once

#include <QObject>

/**
 * @brief ViewModel для параметров выбранного фильтра.
 *
 * Экспортирует в QML свойства параметров для нескольких примерных фильтров
 * (яркость и размытие по Гауссу). Фактическое применение параметров к
 * библиотеке фильтров будет реализовано на следующих этапах.
 */
class FilterParametersViewModel : public QObject
{
    Q_OBJECT

    /**
     * @brief Коэффициент яркости для фильтра BrightnessFilter.
     *
     * Значения > 1.0 увеличивают яркость, < 1.0 уменьшают её.
     */
    Q_PROPERTY(double brightnessFactor READ brightnessFactor WRITE setBrightnessFactor NOTIFY brightnessFactorChanged)

    /**
     * @brief Радиус размытия для фильтра GaussianBlurFilter.
     *
     * Значение должно быть > 0.0.
     */
    Q_PROPERTY(double blurRadius READ blurRadius WRITE setBlurRadius NOTIFY blurRadiusChanged)

    /**
     * @brief Идентификатор текущего фильтра, для которого актуальны параметры.
     *
     * Используется для связи с моделью списка фильтров и PreviewViewModel.
     */
    Q_PROPERTY(QString activeFilterId READ activeFilterId WRITE setActiveFilterId NOTIFY activeFilterIdChanged)

public:
    /**
     * @brief Конструктор ViewModel параметров фильтра.
     * @param parent Родительский QObject.
     */
    explicit FilterParametersViewModel(QObject* parent = nullptr);

    /**
     * @brief Возвращает текущий коэффициент яркости.
     * @return Коэффициент яркости.
     */
    [[nodiscard]] double brightnessFactor() const noexcept;

    /**
     * @brief Устанавливает коэффициент яркости.
     * @param factor Новый коэффициент.
     */
    void setBrightnessFactor(double factor);

    /**
     * @brief Возвращает текущий радиус размытия.
     * @return Радиус размытия.
     */
    [[nodiscard]] double blurRadius() const noexcept;

    /**
     * @brief Устанавливает радиус размытия.
     * @param radius Новый радиус.
     */
    void setBlurRadius(double radius);

    /**
     * @brief Возвращает идентификатор текущего фильтра.
     * @return Идентификатор фильтра.
     */
    [[nodiscard]] QString activeFilterId() const;

    /**
     * @brief Устанавливает идентификатор текущего фильтра.
     * @param filterId Новый идентификатор.
     */
    void setActiveFilterId(const QString& filterId);

signals:
    /**
     * @brief Сигнал об изменении коэффициента яркости.
     * @param factor Новый коэффициент.
     */
    void brightnessFactorChanged(double factor);

    /**
     * @brief Сигнал об изменении радиуса размытия.
     * @param radius Новый радиус.
     */
    void blurRadiusChanged(double radius);

    /**
     * @brief Сигнал об изменении идентификатора активного фильтра.
     * @param filterId Новый идентификатор.
     */
    void activeFilterIdChanged(const QString& filterId);

private:
    double brightness_factor_ = 1.2;
    double blur_radius_ = 5.0;
    QString active_filter_id_;
};


