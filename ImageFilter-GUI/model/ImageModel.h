#pragma once

#include <QObject>
#include <QString>
#include <memory>
#include <mutex>

// Forward declaration
class ImageProcessor;

class ImageModel : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор модели изображения
     * @param parent Родительский объект Qt
     */
    explicit ImageModel(QObject* parent = nullptr);

    /**
     * @brief Деструктор
     */
    ~ImageModel() override;

    /**
     * @brief Получает указатель на ImageProcessor
     * @return Указатель на ImageProcessor или nullptr, если изображение не загружено
     */
    [[nodiscard]] const ImageProcessor* getImageProcessor() const noexcept;

    /**
     * @brief Получает указатель на исходный ImageProcessor (неизмененное изображение)
     * @return Указатель на исходный ImageProcessor или nullptr, если изображение не загружено
     */
    [[nodiscard]] const ImageProcessor* getSourceImageProcessor() const noexcept;

    void loadImage(const QString& filePath, bool preserveAlpha = true);
    void saveImage(bool preserveAlpha = true, const QString& filePath = QString());
    
    void applyFilteredImage(const std::shared_ptr<ImageProcessor>& image);

    /**
     * @brief Проверяет, загружено ли изображение
     * @return true если изображение загружено
     */
    [[nodiscard]] bool isImageLoaded() const noexcept;

    /**
     * @brief Получает путь к исходному файлу
     * @return Путь к исходному файлу или пустая строка
     */
    [[nodiscard]] QString getSourceFilePath() const noexcept;

signals:
    void imageLoaded(const ImageProcessor* image);
    void imageSaved();

    /**
     * @brief Сигнал об ошибке
     * @param message Сообщение об ошибке
     */
    void errorOccurred(const QString& message);

private:
    std::unique_ptr<ImageProcessor> imageProcessor_;    ///< Обработчик изображений (текущее состояние)
    std::unique_ptr<ImageProcessor> sourceImageProcessor_;    ///< Исходное изображение (неизмененное)
    QString sourceFilePath_;                            ///< Путь к исходному файлу
    std::mutex imageProcessorMutex_;
};

