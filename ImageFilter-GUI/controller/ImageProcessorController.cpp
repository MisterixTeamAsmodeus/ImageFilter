#include <ImageProcessor.h>
#include <QDebug>
#include <QTimer>
#include <controller/ImageProcessorController.h>
#include <model/FilterChainModel.h>
#include <model/ImageModel.h>
#include <worker/ImageProcessingWorker.h>

ImageProcessorController::ImageProcessorController(FilterChainModel* filterChainModel, ImageModel* imageModel,
                                                   QObject* parent)
    : QObject(parent), filterChainModel_(filterChainModel), imageModel_(imageModel),
      worker_(std::make_unique<ImageProcessingWorker>()) {
    if (filterChainModel_ == nullptr || imageModel_ == nullptr) {
        qWarning() << "ImageProcessorController: filterChainModel или imageModel равен nullptr";
        return;
    }

    // Подключаемся к сигналам модели цепочки фильтров
    connect(filterChainModel_, &FilterChainModel::chainChanged, this, &ImageProcessorController::onChainChanged);
    connect(filterChainModel_, &FilterChainModel::filterParameterChanged, this,
            &ImageProcessorController::onChainChanged);

    // Подключаемся к сигналам модели изображения
    connect(imageModel_, &ImageModel::imageLoaded, this, &ImageProcessorController::onImageModelLoaded,
            Qt::QueuedConnection);
    connect(imageModel_, &ImageModel::imageSaved, this, &ImageProcessorController::onImageModelSaved,
            Qt::QueuedConnection);
    connect(imageModel_, &ImageModel::errorOccurred, this, &ImageProcessorController::onErrorOccurred,
            Qt::QueuedConnection);

    // Подключаемся к сигналам рабочего потока
    connect(worker_.get(), &ImageProcessingWorker::processingFinished, this,
            &ImageProcessorController::onProcessingFinished, Qt::QueuedConnection);
    connect(worker_.get(), &ImageProcessingWorker::processingProgress, this,
            &ImageProcessorController::onProcessingProgress, Qt::QueuedConnection);
    connect(worker_.get(), &ImageProcessingWorker::errorOccurred, this, &ImageProcessorController::onErrorOccurred,
            Qt::QueuedConnection);
}

ImageProcessorController::~ImageProcessorController() = default;

void ImageProcessorController::loadImage(const QString& filePath, bool preserveAlpha) {
    if (filePath.isEmpty()) {
        emit errorOccurred("Путь к файлу пуст");
        return;
    }

    // Делегируем загрузку в ImageModel
    imageModel_->loadImage(filePath, preserveAlpha);
}

void ImageProcessorController::applyFilterChain() {
    if (!imageModel_->isImageLoaded()) {
        emit errorOccurred("Изображение не загружено");
        return;
    }

    if (!filterChainModel_->isValid()) {
        emit errorOccurred("Цепочка фильтров невалидна");
        return;
    }

    // Получаем исходное изображение для обработки
    const auto* sourceProcessor = imageModel_->getSourceImageProcessor();
    if (sourceProcessor == nullptr || !sourceProcessor->isValid()) {
        emit errorOccurred("Исходное изображение не загружено");
        return;
    }

    if (worker_->isProcessing()) {
        worker_->cancelProcessing();
        worker_->waitProcessingEnd();
    }

    // Отправляем сигнал о начале обработки
    emit processingStarted();

    // Применяем цепочку фильтров к исходному изображению
    worker_->applyFilterChain(filterChainModel_, sourceProcessor);
}

void ImageProcessorController::saveImage(const QString& filePath, bool preserveAlpha) {
    if (filePath.isEmpty()) {
        emit errorOccurred("Путь к файлу пуст");
        return;
    }

    if (!imageModel_->isImageLoaded()) {
        emit errorOccurred("Изображение не загружено");
        return;
    }

    // Делегируем сохранение в ImageModel
    imageModel_->saveImage(preserveAlpha, filePath);
}

void ImageProcessorController::waitProcessingEnd() {
    worker_->waitProcessingEnd();
}

void ImageProcessorController::cancelProcessing() {
    worker_->cancelProcessing();
}

void ImageProcessorController::onImageModelLoaded(const ImageProcessor* image) {
    if (image == nullptr || !image->isValid()) {
        emit errorOccurred("Ошибка загрузки изображения");
        return;
    }

    // Конвертируем ImageProcessor в QImage для сигнала
    const QImage qimage = imageProcessorToQImage(image);
    if (qimage.isNull()) {
        emit errorOccurred("Ошибка конвертации изображения");
        return;
    }

    emit imageLoaded(qimage);

    // Автоматически применяем цепочку фильтров после загрузки
    if (filterChainModel_ != nullptr && filterChainModel_->isValid()) {
        applyFilterChain();
    }
}

void ImageProcessorController::onProcessingFinished(const std::shared_ptr<ImageProcessor>& image) {
    // Если image == nullptr, это означает отмену обработки - просто игнорируем
    if (image == nullptr) {
        return;
    }

    if (!image->isValid()) {
        emit errorOccurred("Ошибка обработки изображения");
        return;
    }
    // Конвертируем в QImage для сигнала (используем скопированное изображение)
    const auto qimage = imageProcessorToQImage(image.get());
    if (!qimage.isNull()) {
        emit processingFinished(qimage);
    }
}

void ImageProcessorController::onProcessingProgress(int percent) {
    emit processingProgress(percent);
}

void ImageProcessorController::onErrorOccurred(const QString& message) {
    emit errorOccurred(message);
}

void ImageProcessorController::onImageModelSaved() {
    emit saved();
}

void ImageProcessorController::onChainChanged() {
    // Автоматически применяем цепочку фильтров при изменении
    if (imageModel_ != nullptr && imageModel_->isImageLoaded()) {
        applyFilterChain();
    }
}

QImage ImageProcessorController::imageProcessorToQImage(const ImageProcessor* processor) {
    if (processor == nullptr || !processor->isValid()) {
        return {};
    }

    const int width = processor->getWidth();
    const int height = processor->getHeight();
    const int channels = processor->getChannels();

    if (width <= 0 || height <= 0 || (channels != 3 && channels != 4)) {
        return {};
    }

    // Определяем формат QImage в зависимости от количества каналов
    QImage::Format format = (channels == 4) ? QImage::Format_RGBA8888 : QImage::Format_RGB888;
    const int bytesPerLine = width * channels;

    // Создаем QImage с данными из ImageProcessor
    QImage qimage(processor->getData(), width, height, bytesPerLine, format);

    // Создаем глубокую копию, так как данные ImageProcessor могут быть освобождены
    return qimage.copy();
}
