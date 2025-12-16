#include <ImageProcessor.h>
#include <QDebug>
#include <QThread>
#include <filesystem>
#include <filters/IFilter.h>
#include <model/FilterChainModel.h>
#include <utils/BufferPool.h>
#include <worker/FilterAdapter.h>
#include <worker/ImageProcessingWorker.h>

ImageProcessingWorker::ImageProcessingWorker(QObject* parent)
    : QObject(parent), bufferPool_(std::make_unique<BufferPool>()) {}

ImageProcessingWorker::~ImageProcessingWorker() {
    // Отменяем обработку и ждем завершения потока перед уничтожением
    std::lock_guard<std::mutex> lock(threadMutex_);
    if (thread_ != nullptr) {
        // Отключаем сигнал finished, чтобы избежать проблем при удалении
        disconnect(finishedConnection_);

        needCancel_ = true;
        thread_->requestInterruption();
        // Для потоков, созданных через QThread::create(), quit() не работает,
        // так как они не используют event loop. Просто ждем завершения лямбды.
        thread_->wait();

        // Удаляем поток синхронно, так как мы в деструкторе
        delete thread_;
        thread_ = nullptr;
    }
}

void ImageProcessingWorker::waitProcessingEnd() const {
    std::lock_guard<std::mutex> lock(threadMutex_);
    if (thread_ == nullptr) {
        return;
    }

    thread_->wait();
}

bool ImageProcessingWorker::isProcessing() const {
    std::lock_guard<std::mutex> lock(threadMutex_);
    return thread_ != nullptr && thread_->isRunning();
}

void ImageProcessingWorker::cancelProcessing() {
    QThread* threadToDelete = nullptr;
    {
        std::lock_guard<std::mutex> lock(threadMutex_);
        if (thread_ == nullptr) {
            return;
        }

        // Отключаем сигнал finished
        QObject::disconnect(finishedConnection_);

        needCancel_ = true;
        thread_->requestInterruption();
        // Для потоков, созданных через QThread::create(), quit() не работает,
        // так как они не используют event loop. Просто ждем завершения лямбды.
        thread_->wait();

        // Сохраняем указатель для удаления вне мьютекса
        threadToDelete = thread_;
        thread_ = nullptr;
        needCancel_ = false;
    }

    // Удаляем поток вне мьютекса
    delete threadToDelete;

    emit processingFinished(nullptr);
}

void ImageProcessingWorker::applyFilterChain(const FilterChainModel* chainModel, const ImageProcessor* imageProcessor) {

    if (imageProcessor == nullptr || !imageProcessor->isValid()) {
        emit errorOccurred("Изображение не загружено");
        return;
    }

    if (chainModel == nullptr) {
        emit errorOccurred("Модель цепочки фильтров не инициализирована");
        return;
    }

    if (!chainModel->isValid()) {
        emit errorOccurred("Цепочка фильтров невалидна");
        return;
    }

    if (isProcessing()) {
        emit errorOccurred("Обработка уже в процессе");
        return;
    }

    // Делаем копию списка фильтров в главном потоке, чтобы избежать race condition
    // при изменении цепочки фильтров из UI во время обработки
    const auto filtersCopy = chainModel->getFilters();

    if (filtersCopy.empty()) {
        emit errorOccurred("Цепочка фильтров пуста");
        return;
    }

    // Копируем данные изображения в главном потоке
    auto temp_procesor = std::make_shared<ImageProcessor>();
    temp_procesor->resize(imageProcessor->getWidth(), imageProcessor->getHeight(), imageProcessor->getChannels(),
                          imageProcessor->getData());
    {
        std::lock_guard<std::mutex> lock(threadMutex_);
        needCancel_ = false;
        thread_ = QThread::create([this, filtersCopy, temp_procesor]() {
            // Применяем фильтры последовательно
            for (size_t i = 0; i < filtersCopy.size(); ++i) {
                if (needCancel_.load()) {
                    emit processingFinished(nullptr);
                    return;
                }

                // Отправляем прогресс
                const int percent = static_cast<int>((i * 100) / filtersCopy.size());
                emit processingProgress(percent);

                const auto& filterItem = filtersCopy[i];
                auto filter =
                    FilterAdapter::createFilter(filterItem.filterName, filterItem.parameters, bufferPool_.get());

                if (filter == nullptr) {
                    emit errorOccurred(
                        QString("Неизвестный фильтр: %1").arg(QString::fromStdString(filterItem.filterName)));
                    emit processingFinished(nullptr);
                    return;
                }

                const auto result = filter->apply(*temp_procesor);
                if (!result.isSuccess()) {
                    emit errorOccurred(QString("Ошибка применения фильтра %1: %2")
                                           .arg(QString::fromStdString(filterItem.filterName),
                                                QString::fromStdString(result.getFullMessage())));
                    emit processingFinished(nullptr);
                    return;
                }
            }

            // Отправляем финальный прогресс
            emit processingProgress(100);

            emit processingFinished(temp_procesor);
        });

        // Подключаем сигнал завершения потока для очистки указателя
        finishedConnection_ = connect(
            thread_, &QThread::finished, this,
            [this]() {
                QThread* threadToDelete = nullptr;
                {
                    std::lock_guard<std::mutex> lock(threadMutex_);
                    if (thread_ != nullptr) {
                        threadToDelete = thread_;
                        thread_ = nullptr;
                        // Отключаем соединение, так как поток будет удален
                        finishedConnection_ = QMetaObject::Connection();
                    }
                }
                // Удаляем поток вне мьютекса
                if (threadToDelete != nullptr) {
                    threadToDelete->deleteLater();
                }
            },
            Qt::QueuedConnection);

        thread_->start();
    }
}
