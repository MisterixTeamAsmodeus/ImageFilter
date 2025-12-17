#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QImage>
#include <QMessageBox>
#include <QProgressBar>
#include <QVBoxLayout>

#include <controller/ImageProcessorController.h>
#include <gui/FilterChainPanel.h>
#include <gui/MainWindow.h>
#include <gui/PreviewWidget.h>
#include <model/FilterChainModel.h>
#include <model/ImageModel.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , filterChainPanel_(nullptr)
    , previewWidget_(nullptr)
    , processingProgressBar_(nullptr)
    , filterChainModel_(std::make_unique<FilterChainModel>(this))
    , imageModel_(std::make_unique<ImageModel>(this))
    , controller_(std::make_unique<ImageProcessorController>(filterChainModel_.get(), imageModel_.get(), this))
    , ui_(new Ui::MainWindow())
{
    ui_->setupUi(this);

    // Инициализируем компоненты
    setupMenuBar();
    setupCentralWidget();
    setupStatusBar();

    // Подключаем сигналы контроллера
    connect(controller_.get(), &ImageProcessorController::imageLoaded, this, &MainWindow::onImageLoaded);
    connect(controller_.get(), &ImageProcessorController::processingStarted, this, &MainWindow::onProcessingStarted);
    connect(controller_.get(), &ImageProcessorController::processingFinished, this, &MainWindow::onProcessingFinished);
    connect(controller_.get(), &ImageProcessorController::processingProgress, this, &MainWindow::onProcessingProgress);
    connect(controller_.get(), &ImageProcessorController::errorOccurred, this, &MainWindow::onErrorOccurred);
    connect(controller_.get(), &ImageProcessorController::saved, this, &MainWindow::onSaved);

    // Обновляем состояние меню
    updateMenuState();
}

MainWindow::~MainWindow()
{
    delete ui_;
}

void MainWindow::setupMenuBar()
{
    // Подключаем действия из UI
    connect(ui_->openAction, &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(ui_->saveAction, &QAction::triggered, this, &MainWindow::onSaveFile);
    connect(ui_->saveAsAction, &QAction::triggered, this, &MainWindow::onSaveAsFile);
}

void MainWindow::setupCentralWidget()
{
    // Создаем панели и добавляем их в splitter из UI
    filterChainPanel_ = new FilterChainPanel(filterChainModel_.get(), this);
    previewWidget_ = new PreviewWidget(this);

    filterChainPanel_->setMinimumWidth(260);
    previewWidget_->setMinimumWidth(320);

    ui_->splitter->addWidget(filterChainPanel_);
    ui_->splitter->addWidget(previewWidget_);
    ui_->splitter->setStretchFactor(0, 0); // Левая панель не растягивается
    ui_->splitter->setStretchFactor(1, 1); // Правая панель растягивается
    ui_->splitter->setSizes({300, 900});   // Начальные размеры

    // Открытие диалога выбора файла по двойному клику на пустой области предпросмотра
    if (previewWidget_ != nullptr) {
        connect(previewWidget_, &PreviewWidget::emptyAreaDoubleClicked, this, &MainWindow::onOpenFile);
    }
}

void MainWindow::setupStatusBar()
{
    if (ui_->statusBar == nullptr) {
        return;
    }

    processingProgressBar_ = new QProgressBar(this);
    processingProgressBar_->setRange(0, 100);
    processingProgressBar_->setValue(0);
    processingProgressBar_->setVisible(false);
    processingProgressBar_->setMaximumWidth(200);

    ui_->statusBar->addPermanentWidget(processingProgressBar_);
    ui_->statusBar->showMessage(QStringLiteral("Готово"));
}

void MainWindow::showProcessingStatus(bool success, qint64 durationMs, const QString& message)
{
    if (ui_->statusBar == nullptr) {
        return;
    }

    const double seconds = static_cast<double>(durationMs) / 1000.0;
    const QString durationText = QString::number(seconds, 'f', 1);

    QString statusText;
    if (success) {
        statusText = QStringLiteral("✅ Изображение обработано за %1 сек").arg(durationText);
    } else {
        statusText = QStringLiteral("⚠️ Ошибка обработки");
    }

    if (!message.isEmpty()) {
        statusText.append(QStringLiteral(": ")).append(message);
    }

    ui_->statusBar->showMessage(statusText, 5000);
}

void MainWindow::updateMenuState()
{
    const bool hasImage = imageModel_ != nullptr && imageModel_->isImageLoaded();
    ui_->saveAction->setEnabled(hasImage);
    ui_->saveAsAction->setEnabled(hasImage);
}

QString MainWindow::showOpenFileDialog()
{
    return QFileDialog::getOpenFileName(this, QStringLiteral("Открыть изображение"), QString(),
                                        QStringLiteral("Изображения (*.jpg *.jpeg *.png *.bmp)"));
}

QString MainWindow::showSaveFileDialog(const QString& defaultPath)
{
    return QFileDialog::getSaveFileName(this, QStringLiteral("Сохранить изображение"), defaultPath,
                                        QStringLiteral(
                                            "JPEG файлы (*.jpg *.jpeg);;PNG файлы (*.png);;BMP файлы (*.bmp)"));
}

void MainWindow::onOpenFile()
{
    const QString filePath = showOpenFileDialog();
    if (filePath.isEmpty()) {
        return;
    }

    ui_->statusBar->showMessage(QStringLiteral("Загрузка изображения..."));
    controller_->loadImage(filePath);
}

void MainWindow::onSaveFile() const
{
    ui_->statusBar->showMessage(QStringLiteral("Сохранение изображения..."));
    controller_->saveImage();
}

void MainWindow::onSaveAsFile()
{
    if (imageModel_ == nullptr || !imageModel_->isImageLoaded()) {
        QMessageBox::warning(this, QStringLiteral("Ошибка"), QStringLiteral("Изображение не загружено"));
        return;
    }

    const QString filePath = showSaveFileDialog(imageModel_->getSourceFilePath());
    if (filePath.isEmpty()) {
        return;
    }

    ui_->statusBar->showMessage(QStringLiteral("Сохранение изображения..."));
    controller_->saveImage(filePath);
}

void MainWindow::onImageLoaded(const QImage& image)
{
    if (previewWidget_ != nullptr) {
        previewWidget_->setImage(image);
    }

    ui_->statusBar->showMessage(QStringLiteral("Изображение загружено"), 3000);
    updateMenuState();
}

void MainWindow::onProcessingFinished(const QImage& image) const
{
    if (previewWidget_ != nullptr) {
        previewWidget_->setImage(image);
    }

    if (processingProgressBar_ != nullptr) {
        processingProgressBar_->setVisible(false);
    }

    const qint64 durationMs = processingTimer_.isValid() ? processingTimer_.elapsed() : 0;
    const_cast<MainWindow*>(this)->showProcessingStatus(true, durationMs);
}

void MainWindow::onErrorOccurred(const QString& message)
{
    QMessageBox::critical(this, QStringLiteral("Ошибка"), message);

    if (processingProgressBar_ != nullptr) {
        processingProgressBar_->setVisible(false);
    }

    const qint64 durationMs = processingTimer_.isValid() ? processingTimer_.elapsed() : 0;
    showProcessingStatus(false, durationMs, message);
}

void MainWindow::onSaved() const
{
    ui_->statusBar->showMessage(QStringLiteral("Изображение сохранено"), 3000);
}

void MainWindow::onProcessingStarted() const
{
    if (processingProgressBar_ != nullptr) {
        processingProgressBar_->setVisible(true);
        processingProgressBar_->setRange(0, 0); // Неопределённый прогресс, пока нет процентов
    }

    const_cast<MainWindow*>(this)->processingTimer_.restart();
    ui_->statusBar->showMessage(QStringLiteral("Применение фильтров..."));
}

void MainWindow::onProcessingProgress(int percent) const
{
    if (processingProgressBar_ != nullptr) {
        if (processingProgressBar_->maximum() == 0) {
            processingProgressBar_->setRange(0, 100);
        }
        processingProgressBar_->setValue(percent);
    }

    ui_->statusBar->showMessage(QStringLiteral("Применение фильтров %1%").arg(percent));
}
