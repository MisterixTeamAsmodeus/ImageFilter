#include <gui/PreviewWidget.h>

#include <QEvent>
#include <QMouseEvent>
#include <QPixmap>
#include <QResizeEvent>

PreviewWidget::PreviewWidget(QWidget* parent)
    : QWidget(parent)
    , mainLayout_(nullptr)
    , imageLabel_(nullptr)
{
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(12, 12, 12, 12);
    mainLayout_->setSpacing(0);

    // Создаем метку для отображения изображения или пустой зоны загрузки
    imageLabel_ = new QLabel(this);
    imageLabel_->setAlignment(Qt::AlignCenter);
    imageLabel_->setMinimumSize(200, 200);
    imageLabel_->setScaledContents(false); // Отключаем автоматическое масштабирование

    mainLayout_->addWidget(imageLabel_);
    setLayout(mainLayout_);

    clearImage();
}

void PreviewWidget::setImage(const QImage& image)
{
    if (image.isNull()) {
        clearImage();
        return;
    }

    originalImage_ = image;
    updateImageDisplay();
}

void PreviewWidget::clearImage()
{
    originalImage_ = QImage();
    if (imageLabel_ == nullptr) {
        return;
    }

    imageLabel_->setPixmap(QPixmap());

    // Текст пустой зоны: иконка + подпись курсивом
    QFont font = imageLabel_->font();
    font.setItalic(true);
    font.setPointSize(11);
    imageLabel_->setFont(font);
    imageLabel_->setText(QString::fromUtf8("🖼️\nВыберите изображение"));

    updateEmptyStateAppearance(false);
}

void PreviewWidget::updateImageDisplay()
{
    if (imageLabel_ == nullptr) {
        return;
    }

    if (originalImage_.isNull()) {
        clearImage();
        return;
    }

    // Сбрасываем оформление пустой зоны
    imageLabel_->setText(QString());
    imageLabel_->setFont(QFont());
    imageLabel_->setStyleSheet(QString());

    // Получаем размер доступной области
    const QSize labelSize = imageLabel_->size();
    if (labelSize.width() <= 0 || labelSize.height() <= 0) {
        return;
    }

    // Вычисляем масштабированное изображение с сохранением пропорций
    const QSize imageSize = originalImage_.size();
    const QSize scaledSize = imageSize.scaled(labelSize, Qt::KeepAspectRatio);

    // Создаем масштабированное изображение
    const QPixmap scaledPixmap = QPixmap::fromImage(originalImage_).scaled(
        scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    imageLabel_->setPixmap(scaledPixmap);
}

void PreviewWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateImageDisplay();
}

void PreviewWidget::enterEvent(QEvent* event)
{
    QWidget::enterEvent(event);

    if (!hasImage()) {
        updateEmptyStateAppearance(true);
    }
}

void PreviewWidget::leaveEvent(QEvent* event)
{
    QWidget::leaveEvent(event);

    if (!hasImage()) {
        updateEmptyStateAppearance(false);
    }
}

bool PreviewWidget::hasImage() const
{
    return !originalImage_.isNull();
}

void PreviewWidget::updateEmptyStateAppearance(bool hovered)
{
    if (imageLabel_ == nullptr) {
        return;
    }

    const QString borderColor = hovered ? QStringLiteral("#A0A4AA") : QStringLiteral("#CED4DA");
    const QString backgroundColor = hovered ? QStringLiteral("#F8F9FA") : QStringLiteral("#FFFFFF");

    const QString style = QStringLiteral(
        "QLabel {"
        " background-color: %1;"
        " border: 2px dashed %2;"
        " color: #6C757D;"
        " }").arg(backgroundColor, borderColor);

    imageLabel_->setStyleSheet(style);
}

void PreviewWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    QWidget::mouseDoubleClickEvent(event);

    if (event == nullptr) {
        return;
    }

    // Открываем диалог выбора файла только если изображение не загружено
    if (!hasImage() && event->button() == Qt::LeftButton) {
        emit emptyAreaDoubleClicked();
    }
}


