#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QImage>
#include <QMessageBox>
#include <QVBoxLayout>
#include <controller/ImageProcessorController.h>
#include <gui/FilterChainPanel.h>
#include <gui/MainWindow.h>
#include <gui/PreviewWidget.h>
#include <model/FilterChainModel.h>
#include <model/ImageModel.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), filterChainModel_(std::make_unique<FilterChainModel>(this)),
      imageModel_(std::make_unique<ImageModel>(this)),
      controller_(std::make_unique<ImageProcessorController>(filterChainModel_.get(), imageModel_.get(), this)),
      ui_(new Ui::MainWindow()) {
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

MainWindow::~MainWindow() {
    delete ui_;
}

void MainWindow::setupMenuBar() {
    // Подключаем действия из UI
    connect(ui_->openAction, &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(ui_->saveAction, &QAction::triggered, this, &MainWindow::onSaveFile);
    connect(ui_->saveAsAction, &QAction::triggered, this, &MainWindow::onSaveAsFile);
}

void MainWindow::setupCentralWidget() {
    // Создаем панели и добавляем их в splitter из UI
    filterChainPanel_ = new FilterChainPanel(filterChainModel_.get(), this);
    previewWidget_ = new PreviewWidget(this);

    ui_->splitter->addWidget(filterChainPanel_);
    ui_->splitter->addWidget(previewWidget_);
    ui_->splitter->setStretchFactor(0, 0); // Левая панель не растягивается
    ui_->splitter->setStretchFactor(1, 1); // Правая панель растягивается
    ui_->splitter->setSizes({300, 900});   // Начальные размеры
}

void MainWindow::setupStatusBar() {
    ui_->statusBar->showMessage("Готово");
}

void MainWindow::updateMenuState() {
    const bool hasImage = imageModel_ != nullptr && imageModel_->isImageLoaded();
    ui_->saveAction->setEnabled(hasImage);
    ui_->saveAsAction->setEnabled(hasImage);
}

QString MainWindow::showOpenFileDialog() {
    return QFileDialog::getOpenFileName(this, "Открыть изображение", QString(),
                                        "Изображения (*.jpg *.jpeg *.png *.bmp)");
}

QString MainWindow::showSaveFileDialog(const QString& defaultPath) {
    return QFileDialog::getSaveFileName(this, "Сохранить изображение", defaultPath,
                                        "JPEG файлы (*.jpg *.jpeg);;PNG файлы (*.png);;BMP файлы (*.bmp)");
}

void MainWindow::onOpenFile() {
    const QString filePath = showOpenFileDialog();
    if (filePath.isEmpty()) {
        return;
    }

    ui_->statusBar->showMessage("Загрузка изображения...");
    controller_->loadImage(filePath);
}

void MainWindow::onSaveFile() const {
    ui_->statusBar->showMessage("Сохранение изображения...");
    controller_->saveImage();
}

void MainWindow::onSaveAsFile() {
    if (imageModel_ == nullptr || !imageModel_->isImageLoaded()) {
        QMessageBox::warning(this, "Ошибка", "Изображение не загружено");
        return;
    }

    const QString filePath = showSaveFileDialog(imageModel_->getSourceFilePath());
    if (filePath.isEmpty()) {
        return;
    }

    ui_->statusBar->showMessage("Сохранение изображения...");
    controller_->saveImage(filePath);
}

void MainWindow::onImageLoaded(const QImage& image) {
    if (previewWidget_ != nullptr) {
        previewWidget_->setImage(image);
    }
    ui_->statusBar->showMessage("Изображение загружено", 3000);
    updateMenuState();
}

void MainWindow::onProcessingFinished(const QImage& image) const {
    if (previewWidget_ != nullptr) {
        previewWidget_->setImage(image);
    }

    ui_->statusBar->showMessage("Обработка завершена", 3000);
}

void MainWindow::onErrorOccurred(const QString& message) {
    QMessageBox::critical(this, "Ошибка", message);
    ui_->statusBar->showMessage("Ошибка: " + message, 5000);
}

void MainWindow::onSaved() const {
    ui_->statusBar->showMessage("Изображение сохранено", 3000);
}

void MainWindow::onProcessingStarted() const {
    ui_->statusBar->showMessage("Применение фильтров...");
}

void MainWindow::onProcessingProgress(int percent) const {
    ui_->statusBar->showMessage(QString("Применение фильтров %1%").arg(percent));
}
