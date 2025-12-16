#pragma once

#include <QImage>
#include <QObject>
#include <QString>
#include <memory>
#include <mutex>
#include <atomic>
#include <model/FilterChainModel.h>

// Forward declarations
class ImageProcessor;
class IBufferPool;

/**
 * @brief Рабочий класс для асинхронной обработки изображений в отдельном потоке
 *
 * Выполняется в отдельном потоке (QThread) и предоставляет методы для:
 * - Загрузки изображения из файла
 * - Применения цепочки фильтров
 * - Сохранения изображения в файл
 *
 * Использует сигналы Qt для уведомления UI о результатах операций.
 */
class ImageProcessingWorker : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор рабочего потока
     * @param parent Родительский объект Qt
     */
    explicit ImageProcessingWorker(QObject* parent = nullptr);

    /**
     * @brief Деструктор
     */
    ~ImageProcessingWorker() override;

public:
    /**
     * @brief Применяет цепочку фильтров к изображению
     * @param chainModel Модель цепочки фильтров (не может быть nullptr)
     * @param imageProcessor Обработчик изображения (должен быть загружен)
     */
    void applyFilterChain(const FilterChainModel* chainModel, const ImageProcessor* imageProcessor);

    void waitProcessingEnd() const;

    bool isProcessing() const;

    void cancelProcessing();
signals:
    /**
     * @brief Сигнал о прогрессе обработки
     * @param percent Процент выполнения (0-100)
     */
    void processingProgress(int percent);

    /**
     * @brief Сигнал о завершении обработки
     * @param image Результирующее изображение
     */
    void processingFinished(const std::shared_ptr<ImageProcessor>& image);

    /**
     * @brief Сигнал об ошибке
     * @param message Сообщение об ошибке
     */
    void errorOccurred(const QString& message);

private:
    std::unique_ptr<IBufferPool> bufferPool_;    ///< Пул буферов для оптимизации обработки
    std::atomic<bool> needCancel_{false};        ///< Флаг отмены обработки (потокобезопасный)
    mutable std::mutex threadMutex_;             ///< Мьютекс для синхронизации доступа к thread_
    QThread* thread_ = nullptr;                  ///< Указатель на рабочий поток
    QMetaObject::Connection finishedConnection_;  ///< Соединение сигнала finished для очистки
};

