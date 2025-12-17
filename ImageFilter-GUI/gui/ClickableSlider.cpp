#include <gui/ClickableSlider.h>

#include <QMouseEvent>
#include <QStyle>
#include <QStyleOptionSlider>

ClickableSlider::ClickableSlider(QWidget* parent)
    : QSlider(parent)
{
}

ClickableSlider::ClickableSlider(Qt::Orientation orientation, QWidget* parent)
    : QSlider(orientation, parent)
{
}

void ClickableSlider::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        QStyleOptionSlider opt;
        initStyleOption(&opt);
        const QRect sliderRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
        const QRect grooveRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);

        if (!sliderRect.contains(event->pos()) && grooveRect.isValid())
        {
            int value = 0;
            if (orientation() == Qt::Horizontal)
            {
                const int grooveWidth = grooveRect.width();
                if (grooveWidth > 0)
                {
                    const double ratio = static_cast<double>(event->pos().x() - grooveRect.x()) / grooveWidth;
                    value = minimum() + static_cast<int>(ratio * (maximum() - minimum()));
                }
                else
                {
                    value = minimum();
                }
            }
            else
            {
                const int grooveHeight = grooveRect.height();
                if (grooveHeight > 0)
                {
                    const double ratio = static_cast<double>(event->pos().y() - grooveRect.y()) / grooveHeight;
                    value = minimum() + static_cast<int>((1.0 - ratio) * (maximum() - minimum()));
                }
                else
                {
                    value = minimum();
                }
            }

            value = qBound(minimum(), value, maximum());
            setValue(value);
            event->accept();
            return;
        }
    }

    QSlider::mousePressEvent(event);
}

