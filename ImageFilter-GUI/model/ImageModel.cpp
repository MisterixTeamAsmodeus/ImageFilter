#include <ImageProcessor.h>
#include <model/ImageModel.h>

namespace {
FilterResult copyImageProcessor(const ImageProcessor* source, ImageProcessor* destination) {
    // Устанавливаем данные в целевой ImageProcessor
    return destination->resize(source->getWidth(), source->getHeight(), source->getChannels(), source->getData());
}
} // namespace

ImageModel::ImageModel(QObject* parent)
    : QObject(parent), imageProcessor_(std::make_unique<ImageProcessor>()),
      sourceImageProcessor_(std::make_unique<ImageProcessor>()) {}

ImageModel::~ImageModel() = default;

const ImageProcessor* ImageModel::getImageProcessor() const noexcept {
    return imageProcessor_.get();
}

const ImageProcessor* ImageModel::getSourceImageProcessor() const noexcept {
    return sourceImageProcessor_.get();
}

void ImageModel::loadImage(const QString& filePath, bool preserveAlpha) {
    std::lock_guard<std::mutex> lock(imageProcessorMutex_);
    // Конвертируем путь в локальную 8-битную кодировку для корректной работы с русскими символами в Windows
    auto result = sourceImageProcessor_->loadFromFile(filePath.toLocal8Bit().toStdString(), preserveAlpha);
    if (!result.isSuccess()) {
        emit errorOccurred(QString::fromStdString(result.getFullMessage()));
        return;
    }
    sourceFilePath_ = filePath;
    result = copyImageProcessor(sourceImageProcessor_.get(), imageProcessor_.get());
    if (!result.isSuccess()) {
        emit errorOccurred(QString::fromStdString(result.getFullMessage()));
        return;
    }

    emit imageLoaded(imageProcessor_.get());
}

void ImageModel::saveImage(bool preserveAlpha, const QString& filePath) {
    std::lock_guard<std::mutex> lock(imageProcessorMutex_);
    // Конвертируем путь в локальную 8-битную кодировку для корректной работы с русскими символами в Windows
    const std::string path = filePath.isEmpty() 
        ? sourceFilePath_.toLocal8Bit().toStdString() 
        : filePath.toLocal8Bit().toStdString();
    auto result = imageProcessor_->saveToFile(path, preserveAlpha);
    if (!result.isSuccess()) {
        emit errorOccurred(QString::fromStdString(result.getFullMessage()));
        return;
    }
    if (!filePath.isEmpty()) {
        sourceFilePath_ = filePath;
    }
    emit imageSaved();
}

void ImageModel::applyFilteredImage(const std::shared_ptr<ImageProcessor>& image) {
    std::lock_guard<std::mutex> lock(imageProcessorMutex_);
    auto result = copyImageProcessor(image.get(), imageProcessor_.get());
    if (!result.isSuccess()) {
        emit errorOccurred(QString::fromStdString(result.getFullMessage()));
        return;
    }

    emit imageLoaded(imageProcessor_.get());
}

bool ImageModel::isImageLoaded() const noexcept {
    return imageProcessor_ != nullptr && imageProcessor_->isValid();
}

QString ImageModel::getSourceFilePath() const noexcept {
    return sourceFilePath_;
}