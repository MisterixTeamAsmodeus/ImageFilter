#pragma once

#include <QWidget>
#include <QVariant>
#include <map>
#include <string>

/**
 * @brief Базовый абстрактный виджет параметров фильтра.
 *
 * Класс инкапсулирует общую логику для всех конфигурационных виджетов
 * отдельных фильтров. Конкретные реализации отвечают только за размещение
 * контролов и синхронизацию их значений с моделью параметров фильтра.
 */
class BaseFilterConfigWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор базового виджета параметров фильтра.
     * @param parent Родительский виджет.
     */
    explicit BaseFilterConfigWidget(QWidget* parent = nullptr);

    /**
     * @brief Виртуальный деструктор.
     */
    ~BaseFilterConfigWidget() override;

    /**
     * @brief Устанавливает значения параметров фильтра.
     *
     * Конкретные реализации должны прочитать значения из карты
     * и инициализировать соответствующие элементы управления.
     *
     * @param parameters Карта параметров по имени.
     */
    virtual void setParameters(const std::map<std::string, QVariant>& parameters) = 0;

    /**
     * @brief Возвращает текущие значения параметров фильтра.
     *
     * По умолчанию метод не используется в текущем виджетном UI,
     * но оставлен для будущих сценариев (например, сохранение пресетов
     * или прямое чтение параметров без модели).
     *
     * @return Карта параметров по имени.
     */
    virtual std::map<std::string, QVariant> getParameters() const = 0;

signals:
    /**
     * @brief Сигнал об изменении параметра фильтра пользователем.
     *
     * Конкретные виджеты должны эмитировать этот сигнал при изменении
     * любого контролируемого параметра.
     *
     * @param name Имя параметра.
     * @param value Новое значение параметра.
     */
    void parameterChanged(const std::string& name, const QVariant& value);
};


