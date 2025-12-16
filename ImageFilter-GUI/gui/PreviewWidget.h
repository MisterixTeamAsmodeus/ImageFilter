#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

// Forward declaration
class QResizeEvent;

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
    /**
     * @brief Обновляет отображение изображения с масштабированием
     */
    void updateImageDisplay();

    /**
     * @brief Обработчик изменения размера виджета
     * @param event Событие изменения размера
     */
    void resizeEvent(QResizeEvent* event) override;

    QVBoxLayout* mainLayout_;       ///< Основной layout
    QLabel* imageLabel_;            ///< Метка для отображения изображения
    QImage originalImage_;          ///< Оригинальное изображение
};

