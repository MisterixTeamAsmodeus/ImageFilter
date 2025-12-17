#pragma once

#include <QMainWindow>
#include <QImage>
#include <QString>
#include <QElapsedTimer>

#include <memory>

// Forward declarations
class FilterChainPanel;
class PreviewWidget;
class FilterChainModel;
class ImageModel;
class ImageProcessorController;
class QProgressBar;

namespace Ui
{
    class MainWindow;
}

/**
 * @brief Главное окно приложения
 *
 * Управляет основным интерфейсом приложения, включая меню, панели
 * управления цепочкой фильтров и предпросмотра изображения.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор главного окна
     * @param parent Родительский виджет
     */
    explicit MainWindow(QWidget* parent = nullptr);

    /**
     * @brief Деструктор
     */
    ~MainWindow() override;

private slots:
    /**
     * @brief Обработчик действия "Открыть"
     */
    void onOpenFile();

    /**
     * @brief Обработчик действия "Сохранить"
     */
    void onSaveFile() const;

    /**
     * @brief Обработчик действия "Сохранить как"
     */
    void onSaveAsFile();

    /**
     * @brief Обработчик успешной загрузки изображения
     * @param image Загруженное изображение
     */
    void onImageLoaded(const QImage& image);

    /**
     * @brief Обработчик завершения обработки
     * @param image Результирующее изображение
     */
    void onProcessingFinished(const QImage& image) const;

    /**
     * @brief Обработчик ошибки
     * @param message Сообщение об ошибке
     */
    void onErrorOccurred(const QString& message);

    /**
     * @brief Обработчик успешного сохранения
     */
    void onSaved() const;

    /**
     * @brief Обработчик начала обработки
     */
    void onProcessingStarted() const;

    /**
     * @brief Обработчик прогресса обработки
     * @param percent Процент выполнения (0-100)
     */
    void onProcessingProgress(int percent) const;

private:
    /**
     * @brief Инициализирует меню приложения
     */
    void setupMenuBar();

    /**
     * @brief Инициализирует центральный виджет
     */
    void setupCentralWidget();

    /**
     * @brief Инициализирует статусную строку
     */
    void setupStatusBar();

    /**
     * @brief Отображает статус обработки в статус-баре.
     *
     * @param success Флаг успешности операции.
     * @param durationMs Длительность обработки в миллисекундах.
     * @param message Дополнительное текстовое сообщение (опционально).
     */
    void showProcessingStatus(bool success, qint64 durationMs, const QString& message = QString());

    /**
     * @brief Обновляет состояние меню в зависимости от состояния приложения
     */
    void updateMenuState();

    /**
     * @brief Показывает диалог выбора файла для открытия
     * @return Путь к выбранному файлу или пустая строка
     */
    QString showOpenFileDialog();

    /**
     * @brief Показывает диалог выбора файла для сохранения
     * @param defaultPath Путь по умолчанию
     * @return Путь к файлу для сохранения или пустая строка
     */
    QString showSaveFileDialog(const QString& defaultPath = QString());

    FilterChainPanel* filterChainPanel_;        ///< Панель управления цепочкой фильтров
    PreviewWidget* previewWidget_;              ///< Виджет предпросмотра

    QProgressBar* processingProgressBar_ = nullptr; ///< Индикатор прогресса обработки
    QElapsedTimer processingTimer_;                 ///< Таймер измерения времени обработки

    std::unique_ptr<FilterChainModel> filterChainModel_;    ///< Модель цепочки фильтров
    std::unique_ptr<ImageModel> imageModel_;                ///< Модель изображения
    std::unique_ptr<ImageProcessorController> controller_;  ///< Контроллер обработки
    Ui::MainWindow* ui_;                                   ///< Указатель на UI объект
};

