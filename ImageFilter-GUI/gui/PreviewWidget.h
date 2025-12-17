#pragma once

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

// Forward declarations
class QResizeEvent;
class QMouseEvent;

/**
 * @brief Виджет предпросмотра изображения
 *
 * Отображает исходное или обработанное изображение с автоматическим
 * масштабированием для соответствия размеру виджета.
 */
class PreviewWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор виджета предпросмотра
     * @param parent Родительский виджет
     */
    explicit PreviewWidget(QWidget* parent = nullptr);

    /**
     * @brief Деструктор
     */
    ~PreviewWidget() override = default;

    /**
     * @brief Устанавливает изображение для отображения
     * @param image Изображение для отображения
     */
    void setImage(const QImage& image);

    /**
     * @brief Очищает отображаемое изображение
     */
    void clearImage();

private:
signals:
    /**
     * @brief Сигнал о двойном клике по пустой области предпросмотра.
     *
     * Используется для запроса открытия диалога выбора файла изображения.
     */
    void emptyAreaDoubleClicked();

private:
    /**
     * @brief Обновляет отображение изображения с масштабированием.
     *
     * Если изображение не задано, отображается состояние пустой зоны загрузки.
     */
    void updateImageDisplay();

    /**
     * @brief Обработчик изменения размера виджета.
     * @param event Событие изменения размера
     */
    void resizeEvent(QResizeEvent* event) override;

    /**
     * @brief Обработчик наведения курсора мыши на виджет.
     *
     * Используется для подсветки пустой зоны загрузки изображения.
     *
     * @param event Событие входа.
     */
    void enterEvent(QEvent* event) override;

    /**
     * @brief Обработчик ухода курсора мыши с виджета.
     *
     * Сбрасывает подсветку пустой зоны загрузки изображения.
     *
     * @param event Событие выхода.
     */
    void leaveEvent(QEvent* event) override;

    /**
     * @brief Обработчик двойного клика мышью по виджету.
     *
     * Если изображение отсутствует, генерирует сигнал emptyAreaDoubleClicked().
     *
     * @param event Событие двойного клика.
     */
    void mouseDoubleClickEvent(QMouseEvent* event) override;

    /**
     * @brief Возвращает признак наличия текущего изображения.
     */
    [[nodiscard]] bool hasImage() const;

    /**
     * @brief Обновляет визуальное представление пустой зоны загрузки.
     *
     * @param hovered Флаг, указывающий, находится ли указатель мыши над виджетом.
     */
    void updateEmptyStateAppearance(bool hovered);

    QVBoxLayout* mainLayout_;       ///< Основной layout
    QLabel* imageLabel_;            ///< Метка для отображения изображения
    QImage originalImage_;          ///< Оригинальное изображение
};

