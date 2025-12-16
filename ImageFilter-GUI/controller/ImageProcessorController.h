#pragma once

#include <QObject>
#include <QString>
#include <memory>

// Forward declarations
class FilterChainModel;
class ImageModel;
class ImageProcessingWorker;
class ImageProcessor;
class QImage;

/**
 * @brief Контроллер для координации обработки изображений
 *
 * Координирует работу между моделью цепочки фильтров, моделью изображения
 * и рабочим потоком для асинхронной обработки. Управляет жизненным циклом
 * рабочего потока и обрабатывает сигналы от него.
 */
class ImageProcessorController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор контроллера
     * @param filterChainModel Модель цепочки фильтров
     * @param imageModel Модель изображения
     * @param parent Родительский объект Qt
     */
    explicit ImageProcessorController(
        FilterChainModel* filterChainModel,
        ImageModel* imageModel,
        QObject* parent = nullptr);

    /**
     * @brief Деструктор
     */
    ~ImageProcessorController() override;

    /**
     * @brief Загружает изображение из файла (делегирует в ImageModel)
     * @param filePath Путь к файлу изображения
     * @param preserveAlpha Сохранять ли альфа-канал
     */
    void loadImage(const QString& filePath, bool preserveAlpha = false);

    /**
     * @brief Применяет цепочку фильтров к текущему изображению
     */
    void applyFilterChain();

    /**
     * @brief Ожидает завершения обработки
     */
    void waitProcessingEnd();

    /**
     * @brief Сохраняет текущее изображение в файл (делегирует в ImageModel)
     * @param filePath Путь к файлу для сохранения
     * @param preserveAlpha Сохранять ли альфа-канал
     */
    void saveImage(const QString& filePath = {}, bool preserveAlpha = true);

    /**
     * @brief Отменяет текущую операцию обработки
     */
    void cancelProcessing();

    /**
     * @brief Проверяет, идет ли обработка
     * @return true если идет обработка
     */
    [[nodiscard]] bool isProcessing() const noexcept;

signals:
    /**
     * @brief Сигнал об успешной загрузке изображения
     * @param image Загруженное изображение
     */
    void imageLoaded(const QImage& image);

    /**
     * @brief Сигнал о завершении обработки
     * @param image Результирующее изображение
     */
    void processingFinished(const QImage& image);

    /**
     * @brief Сигнал о начале обработки
     */
    void processingStarted();

    /**
     * @brief Сигнал о прогрессе обработки
     * @param percent Процент выполнения (0-100)
     */
    void processingProgress(int percent);

    /**
     * @brief Сигнал об ошибке
     * @param message Сообщение об ошибке
     */
    void errorOccurred(const QString& message);

    /**
     * @brief Сигнал об успешном сохранении изображения
     */
    void saved();

private slots:
    /**
     * @brief Обработчик успешной загрузки изображения от ImageModel
     * @param image Загруженное изображение
     */
    void onImageModelLoaded(const ImageProcessor* image);

    /**
     * @brief Обработчик завершения обработки от рабочего потока
     * @param image Результирующее изображение
     */
    void onProcessingFinished(const std::shared_ptr<ImageProcessor>& image);

    /**
     * @brief Обработчик прогресса обработки от рабочего потока
     * @param percent Процент выполнения
     */
    void onProcessingProgress(int percent);

    /**
     * @brief Обработчик ошибки от рабочего потока или ImageModel
     * @param message Сообщение об ошибке
     */
    void onErrorOccurred(const QString& message);

    /**
     * @brief Обработчик успешного сохранения от ImageModel
     */
    void onImageModelSaved();

    /**
     * @brief Обработчик изменения цепочки фильтров
     */
    void onChainChanged();

private:
    /**
     * @brief Конвертирует ImageProcessor в QImage для отображения
     * @param processor Указатель на ImageProcessor
     * @return QImage или пустое изображение при ошибке
     */
    static QImage imageProcessorToQImage(const ImageProcessor* processor);

    FilterChainModel* filterChainModel_;    ///< Модель цепочки фильтров
    ImageModel* imageModel_;               ///< Модель изображения
    std::unique_ptr<ImageProcessingWorker> worker_;         ///< Рабочий объект для обработки
};

