#pragma once

#include <QObject>
#include <QString>

class FilterListViewModel;
class FilterParametersViewModel;
class PreviewViewModel;

/**
 * @brief Главный ViewModel приложения ImageFilter GUI.
 *
 * Координирует работу остальных ViewModel-классов:
 * - текущий путь к изображению;
 * - выбор фильтра;
 * - параметры фильтра;
 * - состояние предпросмотра.
 *
 * Экспортируется в QML как корневой объект, через который UI получает доступ
 * к состоянию и операциям приложения.
 */
class AppViewModel : public QObject
{
    Q_OBJECT

    /**
     * @brief Путь к текущему выбранному изображению.
     */
    Q_PROPERTY(QString currentImagePath READ currentImagePath WRITE setCurrentImagePath NOTIFY currentImagePathChanged)

    /**
     * @brief Модель списка доступных фильтров.
     */
    Q_PROPERTY(QObject* filterList READ filterList CONSTANT)

    /**
     * @brief ViewModel параметров текущего фильтра.
     */
    Q_PROPERTY(QObject* filterParameters READ filterParameters CONSTANT)

    /**
     * @brief ViewModel предпросмотра обработанного изображения.
     */
    Q_PROPERTY(QObject* preview READ preview CONSTANT)

public:
    /**
     * @brief Конструктор главного ViewModel приложения.
     * @param parent Родительский QObject.
     */
    explicit AppViewModel(QObject* parent = nullptr);

    /**
     * @brief Возвращает путь к текущему изображению.
     * @return Путь к файлу изображения.
     */
    [[nodiscard]] QString currentImagePath() const;

    /**
     * @brief Устанавливает путь к текущему изображению.
     * @param path Новый путь к файлу.
     */
    void setCurrentImagePath(const QString& path);

    /**
     * @brief Возвращает модель списка фильтров.
     * @return Указатель на FilterListViewModel.
     */
    [[nodiscard]] QObject* filterList() const noexcept;

    /**
     * @brief Возвращает ViewModel параметров фильтра.
     * @return Указатель на FilterParametersViewModel.
     */
    [[nodiscard]] QObject* filterParameters() const noexcept;

    /**
     * @brief Возвращает ViewModel предпросмотра.
     * @return Указатель на PreviewViewModel.
     */
    [[nodiscard]] QObject* preview() const noexcept;

signals:
    /**
     * @brief Сигнал об изменении пути к текущему изображению.
     * @param path Новый путь.
     */
    void currentImagePathChanged(const QString& path);

private:
    QString current_image_path_;
    FilterListViewModel* filter_list_view_model_ = nullptr;
    FilterParametersViewModel* filter_parameters_view_model_ = nullptr;
    PreviewViewModel* preview_view_model_ = nullptr;

    void initializeConnections();
};


