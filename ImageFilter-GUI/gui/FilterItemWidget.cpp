#include <gui/FilterItemWidget.h>

#include <gui/BaseFilterConfigWidget.h>
#include <gui/FilterWidgetFactory.h>

#include "ui_FilterItemWidget.h"

FilterItemWidget::FilterItemWidget(size_t filterIndex, const std::string& filterName,
                                   const std::map<std::string, QVariant>& parameters, bool canRemove, bool canMoveUp,
                                   bool canMoveDown, QWidget* parent)
    : QWidget(parent), filterIndex_(filterIndex), filterName_(filterName), ui_(new Ui::FilterItemWidget()),
      configWidget_() {
    ui_->setupUi(this);

    // Устанавливаем отображаемое имя фильтра
    ui_->filterNameLabel->setText(getFilterDisplayName(filterName_));

    // Инициализируем кнопку сворачивания параметров
    ui_->toggleParametersButton->setCheckable(true);
    ui_->toggleParametersButton->setChecked(true);
    ui_->toggleParametersButton->setArrowType(Qt::DownArrow);
    ui_->toggleParametersButton->setToolTip(tr("Скрыть параметры фильтра"));

    // Подключаем кнопки управления
    connect(ui_->changeTypeButton, &QPushButton::clicked, this, &FilterItemWidget::onChangeFilterTypeClicked);
    connect(ui_->moveUpButton, &QPushButton::clicked, this, &FilterItemWidget::onMoveUpClicked);
    connect(ui_->moveDownButton, &QPushButton::clicked, this, &FilterItemWidget::onMoveDownClicked);
    connect(ui_->removeButton, &QPushButton::clicked, this, &FilterItemWidget::onRemoveClicked);
    connect(ui_->toggleParametersButton, &QToolButton::toggled, this, &FilterItemWidget::onToggleParametersClicked);

    createConfigWidget(filterName, parameters);
    updateButtonsState(canRemove, canMoveUp, canMoveDown);
}

FilterItemWidget::~FilterItemWidget() {
    delete ui_;
}

void FilterItemWidget::createConfigWidget(const std::string& filterName,
                                          const std::map<std::string, QVariant>& parameters) {
    // Очищаем существующие виджеты параметров
    QLayoutItem* item = nullptr;
    while ((item = ui_->parametersLayout->layout()->takeAt(0)) != nullptr) {
        if (item->widget() != nullptr) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    configWidget_ = nullptr;

    FilterWidgetFactory factory;
    auto widget = factory.create(filterName, this);
    if (widget == nullptr) {
        ui_->parametersLayout->setVisible(false);
        ui_->toggleParametersButton->setVisible(false);
        return;
    }

    widget->setParameters(parameters);

    connect(widget, &BaseFilterConfigWidget::parameterChanged, this,
            [this](const std::string& parameterName, const QVariant& value) {
                emit parameterChanged(filterIndex_, parameterName, value);
            });

    ui_->toggleParametersButton->setVisible(true);
    ui_->parametersLayout->setVisible(parametersVisible_);
    ui_->parametersLayout->layout()->addWidget(widget);
    configWidget_ = widget;
}

QString FilterItemWidget::getFilterDisplayName(const std::string& filterName) {
    static const std::map<std::string, QString> filterNames = {{"grayscale", "Оттенки серого"},
                                                               {"sepia", "Сепия"},
                                                               {"invert", "Инверсия"},
                                                               {"brightness", "Яркость"},
                                                               {"contrast", "Контраст"},
                                                               {"saturation", "Насыщенность"},
                                                               {"flip_h", "Отразить по горизонтали"},
                                                               {"flip_v", "Отразить по вертикали"},
                                                               {"rotate90", "Поворот на 90°"},
                                                               {"sharpen", "Повышение резкости"},
                                                               {"edges", "Детекция краёв"},
                                                               {"emboss", "Рельеф"},
                                                               {"outline", "Контур"},
                                                               {"blur", "Размытие по Гауссу"},
                                                               {"box_blur", "Размытие по прямоугольнику"},
                                                               {"motion_blur", "Размытие движения"},
                                                               {"median", "Медианный фильтр"},
                                                               {"noise", "Шум"},
                                                               {"posterize", "Постеризация"},
                                                               {"threshold", "Пороговая бинаризация"},
                                                               {"vignette", "Виньетирование"}};

    const auto it = filterNames.find(filterName);
    if (it != filterNames.end()) {
        return it->second;
    }
    return QString::fromStdString(filterName);
}

void FilterItemWidget::updateParameters(const std::map<std::string, QVariant>& parameters) {
    if (configWidget_ == nullptr) {
        return;
    }
    configWidget_->setParameters(parameters);
}

void FilterItemWidget::onToggleParametersClicked(bool checked) {
    parametersVisible_ = checked;

    ui_->parametersLayout->setVisible(parametersVisible_);

    ui_->toggleParametersButton->setArrowType(parametersVisible_ ? Qt::DownArrow : Qt::RightArrow);
    ui_->toggleParametersButton->setToolTip(parametersVisible_ ? tr("Скрыть параметры фильтра")
                                                               : tr("Показать параметры фильтра"));
}

void FilterItemWidget::updateFilterName(const std::string& filterName) {
    filterName_ = filterName;
    if (ui_ != nullptr && ui_->filterNameLabel != nullptr) {
        ui_->filterNameLabel->setText(getFilterDisplayName(filterName_));
    }
    // Пересоздаем конфигурационный виджет для нового типа фильтра
    createConfigWidget(filterName_, {});
}

void FilterItemWidget::updateButtonsState(bool canRemove, bool canMoveUp, bool canMoveDown) {
    if (ui_ != nullptr && ui_->removeButton != nullptr) {
        ui_->removeButton->setEnabled(canRemove);
    }
    if (ui_ != nullptr && ui_->moveUpButton != nullptr) {
        ui_->moveUpButton->setEnabled(canMoveUp);
    }
    if (ui_ != nullptr && ui_->moveDownButton != nullptr) {
        ui_->moveDownButton->setEnabled(canMoveDown);
    }
}

void FilterItemWidget::onRemoveClicked() {
    emit removeRequested(filterIndex_);
}

void FilterItemWidget::onMoveUpClicked() {
    emit moveUpRequested(filterIndex_);
}

void FilterItemWidget::onMoveDownClicked() {
    emit moveDownRequested(filterIndex_);
}

void FilterItemWidget::onChangeFilterTypeClicked() {
    emit filterTypeChangeRequested(filterIndex_);
}
