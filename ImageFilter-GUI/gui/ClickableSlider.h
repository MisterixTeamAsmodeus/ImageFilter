#pragma once

#include <QSlider>

/**
 * @brief Кастомный слайдер, который устанавливает значение на позицию клика
 *
 * Переопределяет поведение QSlider так, чтобы при клике на трек
 * значение устанавливалось на позицию клика, а не изменялось на один шаг.
 */
class ClickableSlider : public QSlider
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор
     * @param parent Родительский виджет
     */
    explicit ClickableSlider(QWidget* parent = nullptr);

    /**
     * @brief Конструктор с ориентацией
     * @param orientation Ориентация слайдера
     * @param parent Родительский виджет
     */
    explicit ClickableSlider(Qt::Orientation orientation, QWidget* parent = nullptr);

protected:
    /**
     * @brief Обработка события нажатия мыши
     * @param event Событие нажатия мыши
     */
    void mousePressEvent(QMouseEvent* event) override;
};

