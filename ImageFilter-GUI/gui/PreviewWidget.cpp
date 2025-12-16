#include <gui/PreviewWidget.h>
#include <QResizeEvent>
#include <QPixmap>
#include <QScrollArea>

PreviewWidget::PreviewWidget(QWidget* parent)
    : QWidget(parent)
    , mainLayout_(nullptr)
    , imageLabel_(nullptr)
{
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(5, 5, 5, 5);
    mainLayout_->setSpacing(0);

    // Создаем метку для отображения изображения
    imageLabel_ = new QLabel(this);
    imageLabel_->setAlignment(Qt::AlignCenter);
    imageLabel_->setText("Изображение не загружено");
    imageLabel_->setStyleSheet("QLabel { background-color: #e0e0e0; border: 1px solid #ccc; }");
    imageLabel_->setMinimumSize(200, 200);
    imageLabel_->setScaledContents(false); // Отключаем автоматическое масштабирование

    mainLayout_->addWidget(imageLabel_);
    setLayout(mainLayout_);
}

void PreviewWidget::setImage(const QImage& image)
{
    if (image.isNull())
    {
        clearImage();
        return;
    }

    originalImage_ = image;
    updateImageDisplay();
}

void PreviewWidget::clearImage()
{
    originalImage_ = QImage();
    if (imageLabel_ != nullptr)
    {
        imageLabel_->setText("Изображение не загружено");
        imageLabel_->setPixmap(QPixmap());
    }
}

void PreviewWidget::updateImageDisplay()
{
    if (imageLabel_ == nullptr || originalImage_.isNull())
    {
        return;
    }

    // Получаем размер доступной области
    const QSize labelSize = imageLabel_->size();
    if (labelSize.width() <= 0 || labelSize.height() <= 0)
    {
        return;
    }

    // Вычисляем масштабированное изображение с сохранением пропорций
    const QSize imageSize = originalImage_.size();
    QSize scaledSize = imageSize.scaled(labelSize, Qt::KeepAspectRatio);

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

