#pragma once

#include <QObject>
#include <QString>

class QImage;

/**
 * @brief ViewModel для управления предпросмотром обработанного изображения.
 *
 * Отвечает за состояние предпросмотра (занятость, сообщение об ошибке) и
 * предоставляет интерфейс для запуска обновления предпросмотра.
 *
 * Фактическая интеграция с библиотекой фильтров и QML-отображением изображения
 * будет реализована на следующих этапах.
 */
class PreviewViewModel : public QObject
{
    Q_OBJECT

    /**
     * @brief Признак того, что предпросмотр в данный момент пересчитывается.
     */
    Q_PROPERTY(bool busy READ isBusy NOTIFY busyChanged)

    /**
     * @brief Сообщение об ошибке последней операции предпросмотра.
     */
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    /**
     * @brief Конструктор ViewModel предпросмотра.
     * @param parent Родительский QObject.
     */
    explicit PreviewViewModel(QObject* parent = nullptr);

    /**
     * @brief Возвращает признак занятости.
     * @return true, если предпросмотр обновляется, иначе false.
     */
    [[nodiscard]] bool isBusy() const noexcept;

    /**
     * @brief Возвращает сообщение об ошибке.
     * @return Последнее сообщение об ошибке или пустую строку.
     */
    [[nodiscard]] QString errorMessage() const;

public slots:
    /**
     * @brief Запрашивает пересчёт предпросмотра для текущих настроек.
     *
     * Конкретная реализация будет добавлена позднее (синхронная или
     * асинхронная обработка изображения).
     */
    void requestPreviewUpdate();

signals:
    /**
     * @brief Сигнал об изменении признака занятости.
     * @param busy Новый статус.
     */
    void busyChanged(bool busy);

    /**
     * @brief Сигнал об изменении сообщения об ошибке.
     * @param message Новое сообщение.
     */
    void errorMessageChanged(const QString& message);

    /**
     * @brief Сигнал, извещающий об обновлении данных предпросмотра.
     *
     * Конкретный механизм передачи изображения в QML будет добавлен позже
     * (например, через QImage/QQuickImageProvider).
     */
    void previewUpdated();

private:
    bool busy_ = false;
    QString error_message_;
};


